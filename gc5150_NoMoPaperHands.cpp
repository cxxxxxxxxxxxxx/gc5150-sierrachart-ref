#include "sierrachart.h"
SCDLLName("gc5150 - Trading: Lock Trading Until Position Closed")
const SCString ContactInformation = "gc5150, @gc5150 (twitter)";

/*==============================================================================
	This study continually locks trading until position is closed. Does have
	iisues though due to how client side works. Mainly a 'for fun' Study. You
	can always be extra super paper hand and just remove the study and then
	unlock trading :)
	https://www.sierrachart.com/SupportBoard.php?ThreadID=73330
------------------------------------------------------------------------------*/
SCSFExport scsf_NoMoPaperHands(SCStudyInterfaceRef sc)
{
	SCInputRef Input_Enabled = sc.Input[0];
	SCInputRef Input_AutoLockUnlock = sc.Input[1];
	SCInputRef Input_CancelAllOrders = sc.Input[2];
	SCInputRef Input_FlattenAndCancelAllOrders = sc.Input[3];
	SCInputRef Input_EnableDebuggingOutput = sc.Input[4];

	SCString Msg;
	int &ActiveTrade = sc.GetPersistentIntFast(1);

	if (sc.SetDefaults)
	{
		sc.GraphName = "Trading: Lock Trading Until Position Closed";

		sc.AutoLoop = 0; // manual looping
		sc.GraphRegion = 0;
		sc.CalculationPrecedence = LOW_PREC_LEVEL;

		Input_Enabled.Name = "Enabled";
		Input_Enabled.SetYesNo(false);

		Input_AutoLockUnlock.Name = "Auto Lock/Unlock based on positions";
		Input_AutoLockUnlock.SetYesNo(true);

		Input_CancelAllOrders.Name = "Cancel All Orders - After Stop or Target Hit";
		Input_CancelAllOrders.SetYesNo(true);

		Input_FlattenAndCancelAllOrders.Name = "Flatten And Cancel All Orders - After Stop or Target Hit";
		Input_FlattenAndCancelAllOrders.SetYesNo(false);

		Input_EnableDebuggingOutput.Name = "Enable Debugging Output";
		Input_EnableDebuggingOutput.SetYesNo(false);

		// Not sure if needed. Don't send orders if on SIM. Otherwise send them to live
		sc.SendOrdersToTradeService = !sc.GlobalTradeSimulationIsOn;

		return;
	}

	// TODO: https://www.sierrachart.com/index.php?page=doc/GeneralSettings.html#NeverAutomaticallyOpenMessageLogOrTradeServiceLog
	// Trade-> Auto Trading Enabled (required)

	// Return if Study not enabled, nothing to do
	if (!Input_Enabled.GetYesNo())
		return;

	// Get Position data for the symbol that this trading study is applied to.
	s_SCPositionData PositionData;
	int Result = sc.GetTradePosition(PositionData);
	int Quantity = PositionData.PositionQuantity; // Access the quantity

	// if (PositionData.PositionQuantityWithAllWorkingOrders > 0)
	if (Quantity != 0)
	{
		// We are in a position, lock'r up
		sc.SetTradingLockState(1);
		ActiveTrade = 1; // Set active trade flag

		if (Input_EnableDebuggingOutput.GetYesNo())
		{
			Msg.Format("You have positions open, locking trade activity: %d", Quantity);
			sc.AddMessageToLog(Msg, 0);
		}
	}
	else
	{
		// Qty must be zero, so check to see if we should auto unlock now
		if (Input_AutoLockUnlock.GetYesNo())
		{
			sc.SetTradingLockState(0);
			if (Input_EnableDebuggingOutput.GetYesNo())
			{
				Msg.Format("Looks like zero positions, auto unlocking: %d", Quantity);
				sc.AddMessageToLog(Msg, 0);
			}
		}

		// If active trade then also cancel all based on user options
		if (ActiveTrade == 1)
		{
			if (Input_CancelAllOrders.GetYesNo())
			{
				sc.CancelAllOrders();
				if (Input_EnableDebuggingOutput.GetYesNo())
				{
					Msg.Format("Looks like zero positions and previous trade, cancel all orders");
					sc.AddMessageToLog(Msg, 0);
				}
			}
			if (Input_FlattenAndCancelAllOrders.GetYesNo())
			{
				sc.FlattenAndCancelAllOrders();
				if (Input_EnableDebuggingOutput.GetYesNo())
				{
					Msg.Format("Looks like zero positions and previous trade, flatten and cancel all orders");
					sc.AddMessageToLog(Msg, 0);
				}
			}
			ActiveTrade = 0; // Reset flag
		}
	}
}