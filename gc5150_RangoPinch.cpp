#include "sierrachart.h"
SCDLLName("gc5150 - Rango Pinch")
const SCString ContactInformation = "gc5150, @gc5150 (twitter)";

/*=====================================================================
	Rango Pinch - This study is an attempt at ADX/MACD pinch
----------------------------------------------------------------------*/
SCSFExport scsf_RangoPinch(SCStudyInterfaceRef sc)
{

	// Inputs
	enum Input
	{
		MACDThreshold,
		ADXThreshold,
		PinchLength
	};

	// Subgraphs
	enum Graph
	{
		Pinch,
		PinchRelease,
		MACD,
		ADX
	};

	// Settings for these taken from other studies
	// sc.BaseData[] and sc.BaseDataIn[] are both the same. They just have different names referring to the same array of arrays.
	// https://www.sierrachart.com/index.php?page=doc/ACSIL_Members_Variables_And_Arrays.html#scBaseDataIn
	sc.MACD(sc.BaseData[SC_LAST], sc.Subgraph[Graph::MACD], sc.Index, 12, 26, 9, MOVAVGTYPE_EXPONENTIAL);
	sc.ADX(sc.BaseDataIn, sc.Subgraph[Graph::ADX], sc.Index, 14, 14);

	if (sc.SetDefaults)
	{
		// Set the configuration and defaults
		sc.GraphName = "Rango Pinch";
		sc.StudyDescription = "Pinch Study based on MACD and ADX";
		sc.GraphRegion = 0;
		sc.AutoLoop = 1;

		sc.Subgraph[Graph::Pinch].Name = "Pinch";
		sc.Subgraph[Graph::Pinch].PrimaryColor = RGB(219, 219, 219);
		sc.Subgraph[Graph::Pinch].DrawStyle = DRAWSTYLE_COLOR_BAR;
		sc.Subgraph[Graph::Pinch].DrawZeros = false;

		sc.Subgraph[Graph::PinchRelease].Name = "Pinch Release";
		sc.Subgraph[Graph::PinchRelease].PrimaryColor = RGB(0, 255, 0);
		sc.Subgraph[Graph::PinchRelease].DrawStyle = DRAWSTYLE_COLOR_BAR;
		sc.Subgraph[Graph::PinchRelease].DrawZeros = false;

		sc.Input[Input::MACDThreshold].Name = "MACD Threshold [< LT]";
		sc.Input[Input::MACDThreshold].SetFloat(-1.0f);

		sc.Input[Input::ADXThreshold].Name = "ADX Threshold [> GT]";
		sc.Input[Input::ADXThreshold].SetFloat(28.0f);
		sc.Input[Input::ADXThreshold].SetFloatLimits(0.0f, 100.0f); // Not sure if it ever goes above this but so far no...

		sc.Input[Input::PinchLength].Name = "Minimum Pinch Length";
		sc.Input[Input::PinchLength].SetInt(4);
		sc.Input[Input::PinchLength].SetIntLimits(1, 14); // ADX Period is 14. Assuming pinches longer than this are worthless anyway

		return;
	}

	// Get enough bars to handle at least the ADX period / Pinch Length
	sc.DataStartIndex = 14;

	// Do data processing
	bool Pinching = sc.Subgraph[Graph::MACD][sc.Index] < sc.Input[Input::MACDThreshold].GetFloat() &&
		sc.Subgraph[Graph::ADX][sc.Index] > sc.Input[Input::ADXThreshold].GetFloat();

	if (Pinching)
	{
		int BarIndex = 0;
		for (int PinchCount = 0, BarIndex = sc.Index; PinchCount < sc.Input[Input::PinchLength].GetInt(); PinchCount++, BarIndex--)
		{
			Pinching = Pinching && sc.Subgraph[Graph::MACD][BarIndex] < sc.Subgraph[Graph::MACD][BarIndex - 1];
			Pinching = Pinching && sc.Subgraph[Graph::ADX][BarIndex] > sc.Subgraph[Graph::ADX][BarIndex - 1];
		}
	}
	sc.Subgraph[Graph::Pinch][sc.Index] = (Pinching) ? 1.0f : 0.0f;

	sc.Subgraph[Graph::PinchRelease][sc.Index] =
		sc.Subgraph[Graph::MACD][sc.Index] > sc.Subgraph[Graph::MACD][sc.Index - 1] &&
		sc.Subgraph[Graph::MACD][sc.Index - 1] > sc.Subgraph[Graph::MACD][sc.Index - 2] &&
		sc.Subgraph[Graph::ADX][sc.Index] < sc.Subgraph[Graph::ADX][sc.Index - 1] &&
		sc.Subgraph[Graph::ADX][sc.Index - 1] < sc.Subgraph[Graph::ADX][sc.Index - 2] &&
		(sc.Subgraph[Graph::Pinch][sc.Index - 1] ||
		 sc.Subgraph[Graph::Pinch][sc.Index - 2] ||
		 sc.Subgraph[Graph::Pinch][sc.Index - 3] ||
		 sc.Subgraph[Graph::Pinch][sc.Index - 4]);
}