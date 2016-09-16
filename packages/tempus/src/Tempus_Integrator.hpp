#ifndef TEMPUS_INTEGRATOR_HPP
#define TEMPUS_INTEGRATOR_HPP

#include "Tempus_Stepper.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_Describable.hpp"
#include "Teuchos_ParameterListAcceptorDefaultBase.hpp"

#include <string>

namespace Tempus {

/** \brief Thyra Base interface for time integrators.
 *  Time integrators are designed to advance the solution from an initial
 *  time, \f$t_0\f$, to a final time, \f$t_f\f$.
 *
 * <b>Design Considerations</b>
 *   - Integrators manage multiple time steps
 *   - Integrators have a single Stepper
 *   - Time-step ramping and startup are handled by TimeStepControl.
 *   - Solution output, e.g., solution plotting and check pointing,
 *     is coordinated in the Integrator.
 *   - Solution stability is handled in the timeStepControl, e.g., CFL
 *     constraint.
 *   - Error control over multiple time steps is handled in the Integrators,
 *     while error control over a single time step is handled in the Steppers.
 *   - Integrators will collect error control information from the Stepper
 *     and determine the next time step size and order.
 *   - Integrator maintains its own copy of the time history in the
 *     SolutionHistory, which may be just a single time step up to
 *     the entire solution history.
 *   - Integrators should compute the next time, and after accepting
 *     the time step advance the solution.  This allows a simple undo
 *     capability, if a solution is not acceptable.
 */
template<class Scalar>
class Integrator
  : virtual public Teuchos::Describable,
    virtual public Teuchos::VerboseObject<Tempus::Integrator<Scalar> >,
    virtual public Teuchos::ParameterListAcceptor
{
public:

  /// \name Basic integrator methods
  //@{
    /// Advance the solution to time, and return true if successful.
    virtual bool advanceTime(const Scalar time_final) = 0;
    /// Get current time
    virtual Scalar getTime() const = 0;
    /// Get current index
    virtual Scalar getIndex() const = 0;
    /// Get the stepper
    virtual Teuchos::RCP<Stepper<Scalar> > getStepper() const = 0;
  //@}

};
} // namespace Tempus
#endif // TEMPUS_INTEGRATOR_HPP
