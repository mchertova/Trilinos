// $Id$ 
// $Source$ 
// @HEADER
// ***********************************************************************
// 
//                           Sacado Package
//                 Copyright (2004) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact David M. Gay (dmgay@sandia.gov) or Eric T. Phipps
// (etphipp@sandia.gov).
// 
// ***********************************************************************
// @HEADER

#ifndef CONSTANTDIRICHLETBC_HPP
#define CONSTANTDIRICHLETBC_HPP

#include "AbstractBC.hpp"

class ConstantDirichletBC : public AbstractBC {
public:

  //! Constructor
  ConstantDirichletBC(unsigned int dof_GID, double value);

  //! Destructor
  virtual ~ConstantDirichletBC();

    //! Apply boundary condition to residual
  virtual void applyResidual(const Epetra_Vector& x, 
			     Epetra_Vector& f) const;

  //! Apply boundary condition to Jacobian
  virtual void applyJacobian(const Epetra_Vector& x, Epetra_Vector& f,
			     Epetra_CrsMatrix& jac) const;

private:

  //! Private to prohibit copying
  ConstantDirichletBC(const ConstantDirichletBC&);

  //! Private to prohibit copying
  ConstantDirichletBC& operator=(const ConstantDirichletBC&);

protected:

  //! GID of DOF
  unsigned int dof;

  //! Value of BC
  double val;

};

#endif // CONSTANTDIRICHLETBC_HPP
