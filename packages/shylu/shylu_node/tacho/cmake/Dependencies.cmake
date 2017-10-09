SET(LIB_REQUIRED_DEP_PACKAGES Kokkos)
SET(LIB_OPTIONAL_DEP_PACKAGES TrilinosSS Gtest)
SET(TEST_REQUIRED_DEP_PACKAGES Kokkos TrilinosSS Gtest)
SET(TEST_OPTIONAL_DEP_PACKAGES)
SET(LIB_REQUIRED_DEP_TPLS METIS)
SET(LIB_OPTIONAL_DEP_TPLS Scotch Cholmod HWLOC HYPRE MKL LAPACK BLAS Pthread QTHREAD VTune)
SET(TEST_REQUIRED_DEP_TPLS BLAS LAPACK METIS)
SET(TEST_OPTIONAL_DEP_TPLS HWLOC MKL LAPACK BLAS Pthread QTHREAD)