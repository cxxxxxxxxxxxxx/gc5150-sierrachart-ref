#include "sierrachart.h"
SCDLLName("gc5150 - Output Bar Formula To Subgraph")
const SCString ContactInformation = "gc5150, @gc5150 (twitter)";

/*==============================================================================
	This study outputs the result of a formula to subgraph
------------------------------------------------------------------------------*/
SCSFExport scsf_OutputBarFormulaToSubgraph(SCStudyInterfaceRef sc)
{
	if (sc.SetDefaults)
	{
		sc.GraphName = "Output Bar Formula To Subgraph";
		sc.GraphRegion = 2;
		sc.AutoLoop = 0; // manual looping

		sc.Input[0].Name = "Formula";
		sc.Input[0].SetString("=IF(V>100, V/2, 0)");

		sc.Subgraph[0].Name = "Formula Result";
		sc.Subgraph[0].DrawStyle = DRAWSTYLE_STAIR_STEP;
		sc.Subgraph[0].DrawZeros = true;

		return;
	}

	const char* AlertFormula = sc.Input[0].GetString();
	
	// Update formula only on full recalc
	bool ParseAndSetFormula = sc.IsFullRecalculation;

	for (int BarIndex = sc.UpdateStartIndex; BarIndex < sc.ArraySize; BarIndex++)
	{
		double FormulaResult = sc.EvaluateGivenAlertConditionFormulaAsDouble(BarIndex, ParseAndSetFormula, AlertFormula);
		ParseAndSetFormula = false;

		sc.Subgraph[0][BarIndex] = (float)FormulaResult;
	}
}