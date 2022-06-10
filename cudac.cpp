
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//错误处理函数
static void HandleError( cudaError_t err,
                         const char *file,
                         int line ) {
    //判断判断函数调用是否返回了一个错误值，若为错误则退出程序并输出错误内容
    if (err != cudaSuccess) {
        printf( "%s in %s at line %d\n", cudaGetErrorString( err ),
                file, line );
        exit( EXIT_FAILURE );
    }
}
//此处定义了一个宏，由于判断函数调用是否返回了一个错误值
#define HANDLE_ERROR( err ) (HandleError( err, __FILE__, __LINE__ ))
//此处定义了一个用于判断函数是否返回空值的函数
#define HANDLE_NULL( a ) {if (a == NULL) { \
                            printf( "Host memory failed in %s at line %d\n", \
                                    __FILE__, __LINE__ ); \
                            exit( EXIT_FAILURE );}}

const int N = 1024*1024*64;     	//积分时划分的份数
const int threadsPerBlock = 256;	//block中的线程数 对应为SP
const int blocksPerGrid = 64;		//grid中的block数，对应为SM

//CPU运行，CPU调用
double function_for_cpu(double x) {
    return 4/(1+x*x);
}

//GPU运行，GPU调用，其中__device__函数前缀用于指定此函数在设备上执行，且仅可以通过设备调用
__device__ double function_for_gpu(double x) {
    return 4/(1+x*x);
}

//GPU运行，CPU调用，其中__global__函数前缀用于将函数生命为内核函数
//核函数为cuda运行主要函数，用于将任务分配从而并行执行
__global__ void trap(double *a, double *b, double *integral ) {
    //初始化单个SM的共享内存，此处创建单个SM中线程数量相同数量大小的双精度数组
    // 用于存放每个线程的结果，减少各线程之间的访问冲突与不一致问题
    __shared__ double cache[threadsPerBlock];

    //计算一维索引
    //其中blockDim.x为block在一维上的维度，blockIdx.x * blockDim.x得到当前行前的所有线程数量
    //加上threadIdx.x便得到本献策很难过序号
    //tid用于指定当前线程处理数据子集
    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    //给cacheIndex赋值为当前thread索引，作为线程在共享内存中的访问序号
    int cacheIndex = threadIdx.x;

    //每个线程计算代码，分别计算分配区间的值
    double   x, temp = 0;
    while (tid < N) {
        x = *a + (double)(*b-*a)/N*(tid+0.5) ;
        temp += function_for_gpu(x);
        tid += blockDim.x * gridDim.x;
    }

    //线程将结果保存至SM中对应的共享内存中的地址
    cache[cacheIndex] = temp;

    //__syncthreads()是cuda的内建函数，用于块内线程通信
    //通过使用此方法实现想成的阻塞，当所有线程均到达此指令时再继续运行
    //此处使用此方法是为了保证所有本SM中的线程均计算完毕，防止出现未完成就开始合并答案出现数据为空的情况
    __syncthreads();

    // 规约操作，将本SM中的所有线程处理结果合并
    // 循环将处理序列分为两段，将后一段的所有值加到前一段上，直到段长度为1
    int i = blockDim.x/2;
    while (i != 0) {
        if (cacheIndex < i)
            cache[cacheIndex] += cache[cacheIndex + i];
        //同上，保证单次循环中所有值的合并结束
        __syncthreads();
        i /= 2;
    }

    //将本SM的结果保存到显存上，本SM完成作业
    if (cacheIndex == 0)
        integral[blockIdx.x] = cache[0];
}

//纯CPU处理计算方法，将[0,1]区间切分成N等分并针对每个小区间分别计算一个子结果，并将结果规约
void trap_by_cpu(double a, double b, double *integral) {
    int i;
    //x,temp分别为计算临时结果，结果和
    double x, temp = 0;

    //对每一个区间进行计算，将每个区间的计算结果累加至temp临时变量
    for (i=0; i<N; i++) {
        x = a + (double)(b-a)/N*(i+0.5);
        temp += function_for_cpu(x);
    }
    //结果和除以切分数量，达到放缩效果
    temp *= (double)(b-a)/N;
    //将结果赋值给目标地址，达到返回数据的效果
    *integral = temp;
}

int main( void ) {
    //cpu方法结果保存变量
    double integral;
    //cpu端保存结果，最终通过累加得到最终结果
    double *partial_integral;
    //对应区间上下限
    double a, b;

    //显存保存结果，用于保存每个sm的计算结果，最终将结果转移至外部partial_integral中作为输出
    double   *dev_partial_integral;
    //对应为每个sm上的处理区间上下界限
    double *dev_a, *dev_b;
    //cudaEvent_t，本质是一个GPU时间戳，这个时间戳是在用户指定的时间点上记录的。
    //由于GPU本身支持记录时间戳，因此就避免了当使用CPU定时器来统计GPU执行时间时可能遇到的诸多问题。
    //此处声明的start与stop分别对应cuda任务的开始与结束，用于测量时间
    cudaEvent_t start,stop;
    //tm用于保存gpu时间戳差值，即gpu运行时间
    float tm;

    //clock_t是一个长整形数，在time.h文件中，clock()返回单位是毫秒，使用clock_t可保存时间量
    clock_t  clockBegin, clockEnd;
    //duration用于保存cpu运行时间差，即cpu处理时间
    float duration;

    //cpu处理区间赋值
    a = 0;
    b = 1;

    //第一种方法，仅CPU处理
    //首先获取方法运行前时间戳
    clockBegin = clock();
    //运行cpu计算方法，通过调用trap_by_cpu函数运算，输入区间边界及结果存储变量内存地址用于覆盖结果
    trap_by_cpu(a, b, &integral);
    //获取CPU方法运行结束时间戳
    clockEnd = clock();
    //时间戳相减得到相差时间
    //time.h文件中，定义了一个常量CLOCKS_PER_SEC，它用来表示一秒钟会有多少个时钟计时单元
    //此处通过除以CLOCKS_PER_SEC得到了以秒为单位的cpu程序运行时间
    duration = (float)1000*(clockEnd - clockBegin) / CLOCKS_PER_SEC;
    printf("CPU Result: %.20lf\n", integral);
    printf("CPU Elapsed time: %.6lfms\n", duration );

    //中断操作，用户通过输入进行下一步骤
    getchar();



    //第二种方法，CPU与GPU一起协作
    //cudaEventCreate创建gpu时间戳
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    //cudaEventRecord记录当前时间，GPU将此指令放入其未完成队列中
    // 直到GPU执行完了在调用cudaEventRecord()之前的所有语句时，此事件才会被记录下来
    cudaEventRecord(start,0);

    //申请空间在外部存放各SM处理结果，在GPU处理完成后将显存内容复制到此数组中
    partial_integral = (double*)malloc( blocksPerGrid*sizeof(double) );

    // 分别在显存中申请空间存放输入参数，包括计算区间上下限，结果存储空间
    HANDLE_ERROR( cudaMalloc( (void**)&dev_a, sizeof(double) ) );
    HANDLE_ERROR( cudaMalloc( (void**)&dev_b, sizeof(double) ) );
    HANDLE_ERROR( cudaMalloc( (void**)&dev_partial_integral,
                              blocksPerGrid*sizeof(double) ) );

    //将内存中保存的计算上下限数据拷贝至显存中
    HANDLE_ERROR( cudaMemcpy( dev_a, &a, sizeof(double),cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( dev_b, &b, sizeof(double),cudaMemcpyHostToDevice ) );

    //此处为3. 调用trap函数，将积分所需数据传入，cpu调用陷阱，
    //从而使gpu运行此函数，其中trap函数专门定义global，
    // 使用<<<>>>指定运算单元结构，在本示例中传入1d*1d结构
    //其中blocksPerGrid为SM数量，threadsPerBlock为单个SM中线程数量
    trap<<<blocksPerGrid,threadsPerBlock>>>( dev_a, dev_b, dev_partial_integral );

    //在trap函数结束后将显存中内容拷贝出来保存至内存中，用于下一步规约
    HANDLE_ERROR( cudaMemcpy( partial_integral, dev_partial_integral,
                              blocksPerGrid*sizeof(double),
                              cudaMemcpyDeviceToHost ) );

    //CPU最终将内存中保存的结果数组合并，循环相加
    integral = 0;
    for (int i=0; i<blocksPerGrid; i++) {
        integral += partial_integral[i];
    }
    //最终除以切分数量
    integral *= (double)(b-a)/N;

    //GPU记录时间指令，GPU将此指令放入其任务队列中
    // 且仅当GPU完成了之前的工作并且记录了stop事件后，才能安全地读取stop时间值。
    cudaEventRecord(stop,0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&tm, start, stop);
    printf("GPU Result: %.20lf\n", integral);
    printf("GPU Elapsed time:%.6f ms.\n",tm);

    //将之前申请的显存空间释放
    HANDLE_ERROR( cudaFree( dev_a ) );
    HANDLE_ERROR( cudaFree( dev_b ) );
    HANDLE_ERROR( cudaFree( dev_partial_integral ) );

    //将之前内存申请的结果保存空间释放
    free( partial_integral );

    //中断操作，用户通过输入进行下一步骤，此处为控制程序结束，放置程序结束输出关闭
    getchar();
}
