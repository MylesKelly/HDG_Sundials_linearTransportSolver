#include "SunLinSolWrapper.hpp"
#include <ida/ida.h>                   /* prototypes for IDA fcts., consts.    */
#include <nvector/nvector_serial.h>    /* access to serial N_Vector            */
#include <sunmatrix/sunmatrix_band.h>  /* access to band SUNMatrix             */
#include <sunlinsol/sunlinsol_band.h>  /* access to band SUNLinearSolver       */
#include <sundials/sundials_types.h>   /* definition of type realtype          */

int SunLinSolWrapper::Solve( SUNMatrix A, N_Vector x, N_Vector b )
{
	realtype cj = 1.0;
	N_Vector curY = NULL;
	curY = N_VNew_Serial(2*solver.nCells*(solver.k+1) + solver.nCells + 1);
	IDAGetCurrentY(IDA_mem, &curY);
	IDAGetCurrentCj(IDA_mem, &cj);
	solver.setAlpha(cj);

	if(curY)
	{
		VectorWrapper vec( N_VGetArrayPointer( curY ), N_VGetLength( curY ) );
		std::cerr << vec << std::endl << std::endl;
	}

	solver.solveJacEq( b, x);
	delete curY; 
	return 0;
}

int SunLinSolWrapper::Setup( SUNMatrix mat)
{
	return 0;
}

#define LSWrapper( ls ) reinterpret_cast<SunLinSolWrapper*>( LS->content )

SUNLinearSolver_Type SunLinSolWrapper::LSGetType( SUNLinearSolver LS )
{
	return SUNLINEARSOLVER_DIRECT;
}

SUNLinearSolver_ID SunLinSolWrapper::LSGetID( SUNLinearSolver /* LS */ )
{
	return SUNLINEARSOLVER_CUSTOM;
}

int SunLinSolWrapper::LSinitialize(SUNLinearSolver /* LS */)
{
	return SUNLS_SUCCESS;
}

int SunLinSolWrapper::LSsetup(SUNLinearSolver LS, SUNMatrix M )
{
	int err = LSWrapper( LS )->Setup( M );
	return err;
}

int SunLinSolWrapper::LSsolve(SUNLinearSolver LS, SUNMatrix M, N_Vector x, N_Vector b, realtype)
{
	int err = LSWrapper( LS )->Solve( M, x, b );
	return err;
}

int SunLinSolWrapper::LSfree(SUNLinearSolver LS)
{
	delete LSWrapper( LS );
	return SUNLS_SUCCESS;
}

struct _generic_SUNLinearSolver_Ops LSOps = 
{
	.gettype = SunLinSolWrapper::LSGetType,
	.getid = SunLinSolWrapper::LSGetID,
	.setatimes = nullptr,
	.setpreconditioner = nullptr,
	.setscalingvectors = nullptr,
	.initialize = SunLinSolWrapper::LSinitialize,
	.setup = SunLinSolWrapper::LSsetup,
	.solve = SunLinSolWrapper::LSsolve,
	.numiters = nullptr,
	.resnorm = nullptr,
	.lastflag = nullptr,
	.space = nullptr,
	.resid = nullptr,
	.free = SunLinSolWrapper::LSfree,
};

SUNLinearSolver SunLinSolWrapper::SunLinSol(SystemSolver& solver, void *mem )
{
	SUNLinearSolver LS = SUNLinSolNewEmpty();
	LS->content = new SunLinSolWrapper(solver, mem);
	LS->ops = &LSOps;
	return LS;
}
