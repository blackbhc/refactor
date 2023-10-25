#ifndef _TEST_INFO_H_
#define _TEST_INFO_H_
#ifdef MPI_TEST
#define test_info( ... )                        \
    {                                           \
        int rank;                               \
        MPI_Comm_rank( MPI_COMM_WORLD, &rank ); \
        if ( rank == 0 )                        \
        {                                       \
            printf( __VA_ARGS__ );              \
        }                                       \
    }
#else
#define test_info( ... ) printf( __VA_ARGS__ );
#endif
#endif
