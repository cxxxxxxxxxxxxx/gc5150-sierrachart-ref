#include "sierrachart.h"
SCDLLName("gc5150 - RSI")
const SCString ContactInformation = "gc5150, @gc5150 (twitter)";

/*==============================================================================
	This study is a mod of stock RSI to include additional lines
------------------------------------------------------------------------------*/
SCSFExport scsf_RSI(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Subgraph_RSI = sc.Subgraph[0];
	SCSubgraphRef Subgraph_Line = sc.Subgraph[1];
	SCSubgraphRef Subgraph_Line1 = sc.Subgraph[2];
	SCSubgraphRef Subgraph_Line2 = sc.Subgraph[3];
	SCSubgraphRef Subgraph_Line3 = sc.Subgraph[4];
	SCSubgraphRef Subgraph_Line4 = sc.Subgraph[5];
	SCSubgraphRef Subgraph_RSIAvg = sc.Subgraph[6];

	SCInputRef Input_Data = sc.Input[0];
	SCInputRef Input_RSILength = sc.Input[1];
	SCInputRef Input_RSI_MALength = sc.Input[2];
	SCInputRef Input_Line1Value = sc.Input[3];
	SCInputRef Input_Line2Value = sc.Input[4];
	SCInputRef Input_Line3Value = sc.Input[5];
	SCInputRef Input_Line4Value = sc.Input[6];
	SCInputRef Input_AvgType = sc.Input[7];
	SCInputRef Input_UseRSIMinus50 = sc.Input[8];


	if (sc.SetDefaults)
	{
		sc.GraphName = "RSI";
		sc.GraphRegion = 1;
		sc.ValueFormat = 2;
		sc.AutoLoop = 1;

		Subgraph_RSI.Name = "RSI";
		Subgraph_RSI.DrawStyle = DRAWSTYLE_LINE;
		Subgraph_RSI.LineWidth = 2;
		Subgraph_RSI.PrimaryColor = RGB(13, 166, 240);
		Subgraph_RSI.SecondaryColor = RGB(128, 128, 128);
		Subgraph_RSI.AutoColoring = AUTOCOLOR_SLOPE;
		Subgraph_RSI.DrawZeros = true;
		Subgraph_RSI.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_CENTER | LL_VALUE_ALIGN_VALUES_SCALE;

		Subgraph_RSIAvg.Name = "RSI Avg";
		Subgraph_RSIAvg.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_RSIAvg.PrimaryColor = RGB(255, 127, 0);
		Subgraph_RSIAvg.DrawZeros = true;

		Subgraph_Line.Name = "Line";
		Subgraph_Line.DrawStyle = DRAWSTYLE_LINE;
		Subgraph_Line.PrimaryColor = RGB(255, 255, 157);
		Subgraph_Line.DrawZeros = true;

		Subgraph_Line1.Name = "Line1";
		Subgraph_Line1.DrawStyle = DRAWSTYLE_LINE;
		Subgraph_Line1.PrimaryColor = RGB(128, 128, 128);
		Subgraph_Line1.DrawZeros = true;

		Subgraph_Line2.Name = "Line2";
		Subgraph_Line2.DrawStyle = DRAWSTYLE_LINE;
		Subgraph_Line2.PrimaryColor = RGB(128, 128, 128);
		Subgraph_Line2.DrawZeros = true;

		Subgraph_Line3.Name = "Line3";
		Subgraph_Line3.DrawStyle = DRAWSTYLE_LINE;
		Subgraph_Line3.LineStyle = LINESTYLE_DOT;
		Subgraph_Line3.PrimaryColor = RGB(128, 128, 128);
		Subgraph_Line3.DrawZeros = true;

		Subgraph_Line4.Name = "Line4";
		Subgraph_Line4.DrawStyle = DRAWSTYLE_LINE;
		Subgraph_Line4.LineStyle = LINESTYLE_DOT;
		Subgraph_Line4.PrimaryColor = RGB(128, 128, 128);
		Subgraph_Line4.DrawZeros = true;

		Input_Line1Value.Name = "Line1 Value";
		Input_Line1Value.SetFloat(80.0f);

		Input_Line2Value.Name = "Line2 Value";
		Input_Line2Value.SetFloat(20.0f);

		Input_Line3Value.Name = "Line3 Value";
		Input_Line3Value.SetFloat(70.0f);

		Input_Line4Value.Name = "Line4 Value";
		Input_Line4Value.SetFloat(30.0f);

		Input_Data.Name = "Input Data";
		Input_Data.SetInputDataIndex(SC_LAST);

		Input_RSILength.Name = "RSI Length";
		Input_RSILength.SetInt(14);
		Input_RSILength.SetIntLimits(1, MAX_STUDY_LENGTH);

		Input_RSI_MALength.Name = "RSI Mov Avg Length";
		Input_RSI_MALength.SetInt(3);
		Input_RSI_MALength.SetIntLimits(1, MAX_STUDY_LENGTH);

		Input_UseRSIMinus50.Name = "Use RSI - 50";
		Input_UseRSIMinus50.SetYesNo(0);

		Input_AvgType.Name = "Average Type";
		Input_AvgType.SetMovAvgType(MOVAVGTYPE_SIMPLE);

		return;
	}

	sc.DataStartIndex = (Input_RSILength.GetInt() + Input_RSI_MALength.GetInt());

	sc.RSI(sc.BaseDataIn[Input_Data.GetInputDataIndex()], Subgraph_RSI, Input_AvgType.GetMovAvgType(), Input_RSILength.GetInt());

	Subgraph_Line[sc.Index] = 50.0f;
	Subgraph_Line1[sc.Index] = Input_Line1Value.GetFloat();
	Subgraph_Line2[sc.Index] = Input_Line2Value.GetFloat();
	Subgraph_Line3[sc.Index] = Input_Line3Value.GetFloat();
	Subgraph_Line4[sc.Index] = Input_Line4Value.GetFloat();

	if (Input_UseRSIMinus50.GetYesNo() == 1)
	{
		Subgraph_RSI[sc.Index] = Subgraph_RSI[sc.Index] - 50.0f;
		Subgraph_Line1[sc.Index] = Input_Line1Value.GetFloat() - 50.0f;
		Subgraph_Line2[sc.Index] = Input_Line2Value.GetFloat() - 50.0f;
	}

	sc.MovingAverage(Subgraph_RSI, Subgraph_RSIAvg, Input_AvgType.GetMovAvgType(), Input_RSI_MALength.GetInt());
}