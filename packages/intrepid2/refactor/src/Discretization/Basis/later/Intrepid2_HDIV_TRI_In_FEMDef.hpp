// @HEADER
// ************************************************************************
//
//                           Intrepid2 Package
//                 Copyright (2007) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Kyungjoo Kim  (kyukim@sandia.gov), or
//                    Mauro Perego  (mperego@sandia.gov)
//
// ************************************************************************
// @HEADER

/** \file   Intrepid_HDIV_TRI_In_FEM_Def.hpp
    \brief  Definition file for FEM basis functions (Raviart-Thomas)
            of degree n for H(div) functions on TRI.  
    \author Created by R. Kirby.
*/

namespace Intrepid2 {

  template<class Scalar, class ArrayScalar>
  Basis_HDIV_TRI_In_FEM<Scalar,ArrayScalar>::Basis_HDIV_TRI_In_FEM( const ordinal_type n ,
                                                                    const EPointType pointType ):
    Phis( n ),
    coeffs( (n+1)*(n+2) , n*(n+2) )
  {
    const ordinal_type N = n*(n+2);
    this -> basisCardinality_  = N;
    this -> basisDegree_       = n;
    this -> basisCellTopology_ = shards::CellTopology(shards::getCellTopologyData<shards::Triangle<3> >() );
    this -> basisType_         = BASIS_FEM_FIAT;
    this -> basisCoordinates_  = COORDINATES_CARTESIAN;
    this -> basisTagsAreSet_   = false;


    const ordinal_type littleN = n*(n+1);   // dim of (P_{n-1})^2 -- smaller space
    const ordinal_type bigN = (n+1)*(n+2);  // dim of (P_{n})^2 -- larger space
    const ordinal_type scalarSmallestN = (n-1)*n / 2;
    const ordinal_type scalarLittleN = littleN/2;
    const ordinal_type scalarBigN = bigN/2;

    // first, need to project the basis for RT space onto the
    // orthogonal basis of degree n
    // get coefficients of PkHx

    Teuchos::SerialDenseMatrix<ordinal_type,Scalar> V1(bigN, N);

    // basis for the space is 
    // { (phi_i,0) }_{i=0}^{scalarLittleN-1} ,
    // { (0,phi_i) }_{i=0}^{scalarLittleN-1} ,
    // { (x,y) . phi_i}_{i=scalarLittleN}^{scalarBigN-1}
    // columns of V1 are expansion of this basis in terms of the basis
    // for P_{n}^2

    // these two loops get the first two sets of basis functions
    for (ordinal_type i=0;i<scalarLittleN;i++) {
      V1(i,i) = 1.0;
      V1(scalarBigN+i,scalarLittleN+i) = 1.0;
    }

    // now I need to integrate { (x,y) phi } against the big basis
    // first, get a cubature rule.
    CubatureDirectTriDefault<Scalar,ArrayScalar > myCub( 2 * n );
    ArrayScalar cubPoints( myCub.getNumPoints() , 2 );
    ArrayScalar cubWeights( myCub.getNumPoints() );
    myCub.getCubature( cubPoints , cubWeights );

    // tabulate the scalar orthonormal basis at cubature points
    ArrayScalar phisAtCubPoints( scalarBigN , myCub.getNumPoints() );
    Phis.getValues( phisAtCubPoints , cubPoints , OPERATOR_VALUE );

    // now do the integration
    for (ordinal_type i=0;i<n;i++) {
      for (ordinal_type j=0;j<scalarBigN;j++) { // ordinal_type (x,y) phi_i \cdot (phi_j,0)
        V1(j,littleN+i) = 0.0;
        for (ordinal_type k=0;k<myCub.getNumPoints();k++) {
          V1(j,littleN+i) += 
            cubWeights(k) * cubPoints(k,0) 
            * phisAtCubPoints(scalarSmallestN+i,k) 
            * phisAtCubPoints(j,k);
        }
      }
      for (ordinal_type j=0;j<scalarBigN;j++) {  // ordinal_type (x,y) phi_i \cdot (0,phi_j)
        V1(j+scalarBigN,littleN+i) = 0.0;
        for (ordinal_type k=0;k<myCub.getNumPoints();k++) {
          V1(j+scalarBigN,littleN+i) += 
            cubWeights(k) * cubPoints(k,1) 
            * phisAtCubPoints(scalarSmallestN+i,k) 
            * phisAtCubPoints(j,k);
        }
      }
    }

    //std::cout << V1 << "\n";

    
    // next, apply the RT nodes (rows) to the basis for (P_n)^2 (columns)
    Teuchos::SerialDenseMatrix<ordinal_type,Scalar> V2(N , bigN);

    // first 3 * degree nodes are normals at each edge
    // get the points on the line
    ArrayScalar linePts( n , 1 );
    if (pointType == POINTTYPE_WARPBLEND) {
      CubatureDirectLineGauss<Scalar> edgeRule( n );
      ArrayScalar edgeCubWts( n );
      edgeRule.getCubature( linePts , edgeCubWts );
    }
    else if (pointType == POINTTYPE_EQUISPACED ) {
      shards::CellTopology linetop(shards::getCellTopologyData<shards::Line<2> >() );

      PointTools::getLattice<Scalar,ArrayScalar >( linePts , 
                                                              linetop ,
                                                              n+1 , 1 ,
                                                              POINTTYPE_EQUISPACED );
    }
    // holds the image of the line points 
    ArrayScalar edgePts( n , 2 );
    ArrayScalar phisAtEdgePoints( scalarBigN , n );

    // these are scaled by the appropriate edge lengths.
    const Scalar nx[] = {0.0,1.0,-1.0};
    const Scalar ny[] = {-1.0,1.0,0.0};
    
    for (ordinal_type i=0;i<3;i++) {  // loop over edges
      CellTools<Scalar>::mapToReferenceSubcell( edgePts ,
                                                linePts ,
                                                1 ,
                                                i ,
                                                this->basisCellTopology_ );

      Phis.getValues( phisAtEdgePoints , edgePts , OPERATOR_VALUE );

      // loop over points (rows of V2)
      for (ordinal_type j=0;j<n;j++) {
        // loop over orthonormal basis functions (columns of V2)
        for (ordinal_type k=0;k<scalarBigN;k++) {
          V2(n*i+j,k) = nx[i] * phisAtEdgePoints(k,j);
          V2(n*i+j,k+scalarBigN) = ny[i] * phisAtEdgePoints(k,j);
        }
      }
    }

    // next map the points to each edge


    // remaining nodes are divided into two pieces:  point value of x
    // components and point values of y components.  These are
    // evaluated at the interior of a lattice of degree + 1, For then
    // the degree == 1 space corresponds classicaly to RT0 and so gets
    // no internal nodes, and degree == 2 corresponds to RT1 and needs
    // one internal node per vector component.
    const ordinal_type numInternalPoints = PointTools::getLatticeSize( this->getBaseCellTopology() ,
                                                              n + 1 ,
                                                              1 );

    if (numInternalPoints > 0) {
      ArrayScalar internalPoints( numInternalPoints , 2 );
      PointTools::getLattice<Scalar,ArrayScalar >( internalPoints ,
                                                              this->getBaseCellTopology() , 
                                                              n + 1 ,
                                                              1 ,
                                                              pointType );
      
      ArrayScalar phisAtInternalPoints( scalarBigN , numInternalPoints );
      Phis.getValues( phisAtInternalPoints , internalPoints , OPERATOR_VALUE );
      
      // copy values into right positions of V2
      for (ordinal_type i=0;i<numInternalPoints;i++) {
        for (ordinal_type j=0;j<scalarBigN;j++) {
          // x component
          V2(3*n+i,j) = phisAtInternalPoints(j,i);
          // y component
          V2(3*n+numInternalPoints+i,scalarBigN+j) = phisAtInternalPoints(j,i);
        }
      }
    }
//     std::cout << "Nodes on big basis\n";
//     std::cout << V2 << "\n";
//     std::cout << "End nodes\n";

    Teuchos::SerialDenseMatrix<ordinal_type,Scalar> Vsdm( N , N );

    // multiply V2 * V1 --> V
    Vsdm.multiply( Teuchos::NO_TRANS , Teuchos::NO_TRANS , 1.0 , V2 , V1 , 0.0 );

//     std::cout << "Vandermonde:\n";
//     std::cout << Vsdm << "\n";
//     std::cout << "End Vandermonde\n";
    
    Teuchos::SerialDenseSolver<ordinal_type,Scalar> solver;
    solver.setMatrix( rcp( &Vsdm , false ) );
    solver.invert( );

    Teuchos::SerialDenseMatrix<ordinal_type,Scalar> Csdm( bigN , N );
    Csdm.multiply( Teuchos::NO_TRANS , Teuchos::NO_TRANS , 1.0 , V1 , Vsdm , 0.0 );

    //    std::cout << Csdm << "\n";

    for (ordinal_type i=0;i<bigN;i++) {
      for (ordinal_type j=0;j<N;j++) {
        coeffs(i,j) = Csdm(i,j);
      }
    }

    initializeTags();
    this->basisTagsAreSet_ = true;
  }  
    
  template<class Scalar, class ArrayScalar>
  void Basis_HDIV_TRI_In_FEM<Scalar, ArrayScalar>::initializeTags() {
    // Basis-dependent initializations
    ordinal_type tagSize  = 4;        // size of DoF tag, i.e., number of fields in the tag
    ordinal_type posScDim = 0;        // position in the tag, counting from 0, of the subcell dim 
    ordinal_type posScOrd = 1;        // position in the tag, counting from 0, of the subcell ordinal
    ordinal_type posDfOrd = 2;        // position in the tag, counting from 0, of DoF ordinal relative to the subcell
  
    // An array with local DoF tags assigned to the basis functions, in the order of their local enumeration 
    ordinal_type *tags = new ordinal_type[ tagSize * this->getCardinality() ];
    ordinal_type *tag_cur = tags;
    const ordinal_type degree = this->getDegree();

    // there are degree internal dofs on each edge -- normals.  Let's do them
    for (ordinal_type ed=0;ed<3;ed++) {
      for (ordinal_type i=0;i<degree;i++) {
        tag_cur[0] = 1;  tag_cur[1] = ed;  tag_cur[2] = i;  tag_cur[3] = degree;
        tag_cur += tagSize;
      }
    }
    // end edge dofs

    // the rest of the dofs are internal
    const ordinal_type numFaceDof = (degree-1)*degree;
    ordinal_type faceDofCur = 0;
    for (ordinal_type i=3*degree;i<degree*(degree+2);i++) {
      tag_cur[0] = 2;  tag_cur[1] = 0;  tag_cur[2] = faceDofCur;  tag_cur[3] = numFaceDof;
      tag_cur += tagSize;
      faceDofCur++;
    }
    
    Intrepid2::setOrdinalTagData(this -> tagToOrdinal_,
                                this -> ordinalToTag_,
                                tags,
                                this -> basisCardinality_,
                                tagSize,
                                posScDim,
                                posScOrd,
                                posDfOrd);

    delete []tags;

  }  



  template<class Scalar, class ArrayScalar> 
  void Basis_HDIV_TRI_In_FEM<Scalar, ArrayScalar>::getValues(ArrayScalar &        outputValues,
                                                              const ArrayScalar &  inputPoints,
                                                              const EOperator      operatorType) const {
  
    // Verify arguments
#ifdef HAVE_INTREPID2_DEBUG
    Intrepid2::getValues_HDIV_Args<Scalar, ArrayScalar>(outputValues,
                                                        inputPoints,
                                                        operatorType,
                                                        this -> getBaseCellTopology(),
                                                        this -> getCardinality() );
#endif
    const ordinal_type numPts = inputPoints.dimension(0);
    const ordinal_type deg = this -> getDegree();
    const ordinal_type scalarBigN = (deg+1)*(deg+2)/2;

    try {
      switch (operatorType) {
      case OPERATOR_VALUE:
        {
          ArrayScalar phisCur( scalarBigN , numPts );
          Phis.getValues( phisCur , inputPoints , OPERATOR_VALUE );

          for (ordinal_type i=0;i<outputValues.dimension(0);i++) { // RT bf
            for (ordinal_type j=0;j<outputValues.dimension(1);j++) {  // point
              outputValues(i,j,0) = 0.0;
              outputValues(i,j,1) = 0.0;
              for (ordinal_type k=0;k<scalarBigN;k++) { // Dubiner bf
                outputValues(i,j,0) += coeffs(k,i) * phisCur(k,j);
                outputValues(i,j,1) += coeffs(k+scalarBigN,i) * phisCur(k,j);
              }
            }
          }
        }
        break;
      case OPERATOR_DIV:
        {
          ArrayScalar phisCur( scalarBigN , numPts , 2 );
          Phis.getValues( phisCur , inputPoints , OPERATOR_GRAD );
          for (ordinal_type i=0;i<outputValues.dimension(0);i++) { // bf loop
            for (ordinal_type j=0;j<outputValues.dimension(1);j++) { // point loop
              // dx of x component
              outputValues(i,j) = 0.0;
              for (ordinal_type k=0;k<scalarBigN;k++) {
                outputValues(i,j) += coeffs(k,i) * phisCur(k,j,0);
              }
              // dy of y component
              for (ordinal_type k=0;k<scalarBigN;k++) {
                outputValues(i,j) += coeffs(k+scalarBigN,i) * phisCur(k,j,1);
              }
            }
          }
        }
        break;
      default:
        TEUCHOS_TEST_FOR_EXCEPTION( true , std::invalid_argument,
                            ">>> ERROR (Basis_HDIV_TRI_In_FEM): Operator type not implemented");
        break;
      }
    }
    catch (std::invalid_argument &exception){
      TEUCHOS_TEST_FOR_EXCEPTION( true , std::invalid_argument,
                          ">>> ERROR (Basis_HDIV_TRI_In_FEM): Operator type not implemented");    
    }

  }
  

  
  template<class Scalar, class ArrayScalar>
  void Basis_HDIV_TRI_In_FEM<Scalar, ArrayScalar>::getValues(ArrayScalar&           outputValues,
                                                              const ArrayScalar &    inputPoints,
                                                              const ArrayScalar &    cellVertices,
                                                              const EOperator        operatorType) const {
    TEUCHOS_TEST_FOR_EXCEPTION( (true), std::logic_error,
                        ">>> ERROR (Basis_HDIV_TRI_In_FEM): FEM Basis calling an FVD member function");
  }


}// namespace Intrepid2