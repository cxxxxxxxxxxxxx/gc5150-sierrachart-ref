#include "sierrachart.h"
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

/*
* Study compiled with the following:
* Microsoft Visual Studio Community 2022 (64-bit) - Current
* Version 17.2.2
* 
* Remote builds will fail as it requires an additional non SC header file (json.hpp)
* I'm including the json.hpp file used as I tried an updated version and it failed.
* Hopefully not violating any license issues.
*/

/* JSON Parsing
* https://github.com/nlohmann/json
* Compiled against version 3.10.5
*/
#include "nlohmann/json.hpp" 

SCDLLName("gc5150 - News: Forex Factory Events")
const SCString ContactInformation = "gc5150, @gc5150 (twitter)";

#pragma region Misc Thoughts
/*
* Could create a global data structure to hold settings used in GDI function instead of having all enums
* be global. This may help if you combined this study with other studies in a 'studies DLL' to reduce
* the amount of global items?
* 
* This is the 3rd iteration/re-write of this study as test of splitting design logic into three areas:
* 1. Fetching raw forex events and storing in structure
* 2. Looping through events structure and processing events to push to draw structure
* 3. DrawToChart function only handles drawing what is in the draw structure
* 
* And also a test in organizing Inputs/Graphs/Persistent vars into enums to easily add/remove/re-order
* Learning more on C++ there are probably better ways such as namespaces/classes/etc... but for now...
* 
* ADDED: Showing of Tomorrow's events
* ADDED: The 'All' currency to show things like G7, OPEC, etc...
* 
* FIXED: Hiding events rolling off
* FIXED: All (ALL) events showing up for things like G7, OPEC, etc...
* 
* TODO: Should either UPPER or LOWER call string compares
* TODO: If two events with overlapping times/colors occur, use higher impact color
* TODO: Do we need fill space stuff anymore?
* TODO: I may not be referring to structs/etc... using the correct terminology...
*/
#pragma endregion

#pragma region GDI Documentation
// Windows GDI documentation can be found here: 
// http://msdn.microsoft.com/en-nz/library/windows/desktop/dd145203%28v=vs.85%29.aspx

// Windows GDI font creation
// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta

// Set text colors and alignment
// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-settextalign
#pragma endregion

// SC Drawing function
void DrawToChart(HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc);

// Generate Random Number
// https://cplusplus.com/reference/random/uniform_int_distribution/operator()/
// https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
int RandomInt(int min, int max)
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);

	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(generator);
}

// DateTime independent of replay
SCDateTime GetNow(SCStudyInterfaceRef sc)
{
	if (sc.IsReplayRunning())
		return sc.CurrentDateTimeForReplay;
	else
		return sc.CurrentSystemDateTime;
}

#pragma region Global Data Structure(s)
// This holds events to draw
struct FFEventToDraw {
	SCString EventText;
	COLORREF MarkerColor;
	COLORREF EventTextColor;
	COLORREF EventTextBackgroundColor;
	int FontWeight = 400; // 400 - regular, 700 - bold
	bool StrikeOut = false;
	bool Italic = false;
};
#pragma endregion

#pragma region Enums to Organize sc Inputs/Graphs/Persistent Data
// SC Inputs
enum Input
{
	ExpectedImpactHigh,
	ExpectedImpactMedium,
	ExpectedImpactLow,
	ExpectedImpactNonEconomic,
	CurrencyAUD,
	CurrencyCAD,
	CurrencyCHF,
	CurrencyCNY,
	CurrencyEUR,
	CurrencyGBP,
	CurrencyJPY,
	CurrencyNZD,
	CurrencyUSD,
	CurrencyAll,
	FontSize,
	TimeFormat,
	ExpectedImpactHighColor,
	ExpectedImpactMediumColor,
	ExpectedImpactLowColor,
	ExpectedImpactNonEconomicColor,
	EventTextColor,
	EventTextBackgroundColor,
	UpcomingEventsTextColor,
	UpcomingEventsBackgroundColor,
	CurrentEventsTextColor,
	CurrentEventsBackgroundColor,
	PastEventsTextColor,
	TomorrowEventHeaderMarkerColor,
	TomorrowEventHeaderTextColor,
	TomorrowEventHeaderBackgroundColor,
	ItalicUpcomingEvents,
	BoldCurrentEvents,
	StrikeOutPastEvents,
	HidePastEvents,
	ShowTomorrowsEvents,
	HorizontalOffset,
	VerticalOffset,
	EventSpacingOffset,
	FillSpace,
	RegionsVisible,
	FFUpdateIntervalInMinutes,
};

// SC Subgraphs
enum Graph
{
	UpcomingEventExpectedImpactHigh,
	UpcomingEventExpectedImpactMedium,
	UpcomingEventExpectedImpactLow,
	UpcomingEventExpectedImpactNonEconomic,
	EventHappeningRightNowExpectedImpactHigh,
	EventHappeningRightNowExpectedImpactMedium,
	EventHappeningRightNowExpectedImpactLow,
	EventHappeningRightNowExpectedImpactNonEconomic,
};

// SC Persistent Ints
enum p_Int
{
	UpcomingEvent,
	EventHappeningRightNow,
	RequestState,
};

// SC Persistent Pointers
enum p_Ptr
{
	FFEvents,
	FFEventsToDraw,
	Currencies,
	ExpectedImpact,
};

// SC Persistent DateTime
enum p_DateTime {
	LastFFUpdate,
};

// SC Persistent Strings
enum p_Str
{
	HttpResponseContent,
};
#pragma endregion

/*==============================================================================
	This study loads Forex Factory events onto your chart
------------------------------------------------------------------------------*/
SCSFExport scsf_ForexFactory(SCStudyInterfaceRef sc)
{
	// JSON Parsing
	using json = nlohmann::json;

	// Struct to hold initial raw Event Data
	struct FFEvent {
		SCString title;
		SCString country;
		SCDateTime date;
		SCString impact;
		SCString forecast;
		SCString previous;
	};

	// Track when we last updated study data from Forex Factory
	SCDateTime& LastFFUpdate = sc.GetPersistentSCDateTime(p_DateTime::LastFFUpdate);

	// HTTP Response
	SCString& HttpResponseContent = sc.GetPersistentSCString(p_Str::HttpResponseContent);

	// Pointer to Struct Array for Raw FF Events
	std::vector<FFEvent>* p_FFEvents = reinterpret_cast<std::vector<FFEvent>*>(sc.GetPersistentPointer(p_Ptr::FFEvents));

	// Pointer to Struct Array for FF Events to Draw
	std::vector<FFEventToDraw>* p_FFEventsToDraw = reinterpret_cast<std::vector<FFEventToDraw>*>(sc.GetPersistentPointer(p_Ptr::FFEventsToDraw));

	// Pointer to Struct Array for currencies
	std::vector<SCString>* p_Currencies = reinterpret_cast<std::vector<SCString>*>(sc.GetPersistentPointer(p_Ptr::Currencies));

	// Pointer to Struct Array for Expected Impact Levels
	std::vector<SCString>* p_ExpectedImpact = reinterpret_cast<std::vector<SCString>*>(sc.GetPersistentPointer(p_Ptr::ExpectedImpact));

	// Get current DateTime and H/M/S
	SCDateTime CurrentDateTime = GetNow(sc);
	int CurrentDay = CurrentDateTime.GetDay();
	int CurrentHour = CurrentDateTime.GetHour();
	int CurrentMinute = CurrentDateTime.GetMinute();

	// Generate random second
	// Idea is if many people are using this study then don't have it behave like some bot-net activity
	// with everyone hitting a request update at the exact same time interval
	// Could do more with this probably
	int RandomSecond = RandomInt(0, 29) + RandomInt(30, 59);

	if (sc.SetDefaults)
	{
		sc.GraphName = "News: Forex Factory Events";
		sc.GraphRegion = 0;
		sc.AutoLoop = 0; // manual looping

#pragma region Inputs
		sc.Input[Input::ExpectedImpactHigh].Name = "Expected Impact: High";
		sc.Input[Input::ExpectedImpactHigh].SetYesNo(1);

		sc.Input[Input::ExpectedImpactMedium].Name = "Expected Impact: Medium";
		sc.Input[Input::ExpectedImpactMedium].SetYesNo(1);

		sc.Input[Input::ExpectedImpactLow].Name = "Expected Impact: Low";
		sc.Input[Input::ExpectedImpactLow].SetYesNo(1);

		sc.Input[Input::ExpectedImpactNonEconomic].Name = "Expected Impact: Non-Economic";
		sc.Input[Input::ExpectedImpactNonEconomic].SetYesNo(0);

		sc.Input[Input::CurrencyAUD].Name = "Currencies - AUD";
		sc.Input[Input::CurrencyAUD].SetYesNo(0);
		
		sc.Input[Input::CurrencyCAD].Name = "Currencies - CAD";
		sc.Input[Input::CurrencyCAD].SetYesNo(0);

		sc.Input[Input::CurrencyCHF].Name = "Currencies - CHF";
		sc.Input[Input::CurrencyCHF].SetYesNo(0);

		sc.Input[Input::CurrencyCNY].Name = "Currencies - CNY";
		sc.Input[Input::CurrencyCNY].SetYesNo(0);

		sc.Input[Input::CurrencyEUR].Name = "Currencies - EUR";
		sc.Input[Input::CurrencyEUR].SetYesNo(0);

		sc.Input[Input::CurrencyGBP].Name = "Currencies - GBP";
		sc.Input[Input::CurrencyGBP].SetYesNo(0);

		sc.Input[Input::CurrencyJPY].Name = "Currencies - JPY";
		sc.Input[Input::CurrencyJPY].SetYesNo(0);

		sc.Input[Input::CurrencyNZD].Name = "Currencies - NZD";
		sc.Input[Input::CurrencyNZD].SetYesNo(0);

		sc.Input[Input::CurrencyUSD].Name = "Currencies - USD";
		sc.Input[Input::CurrencyUSD].SetYesNo(1);

		sc.Input[Input::CurrencyAll].Name = "Enable Events with Category 'All' [G7, OPEC, etc..]";
		sc.Input[Input::CurrencyAll].SetYesNo(1);
		
		sc.Input[Input::HorizontalOffset].Name = "Initial Horizontal Position From Left in px";
		sc.Input[Input::HorizontalOffset].SetInt(0);

		sc.Input[Input::VerticalOffset].Name = "Initial Veritical Position From Top in px";
		sc.Input[Input::VerticalOffset].SetInt(1);

		sc.Input[Input::FontSize].Name = "Font Size";
		sc.Input[Input::FontSize].SetInt(18);
		sc.Input[Input::FontSize].SetIntLimits(0, 100);

		sc.Input[Input::FillSpace].Name = "Fill Space for Future Events";
		sc.Input[Input::FillSpace].SetInt(0);
		sc.Input[Input::FillSpace].SetIntLimits(0, MAX_STUDY_LENGTH);

		sc.Input[Input::EventSpacingOffset].Name = "Spacing Between Events";
		sc.Input[Input::EventSpacingOffset].SetInt(2);
		sc.Input[Input::EventSpacingOffset].SetIntLimits(0, 100);

		sc.Input[Input::TimeFormat].Name = "Time Format";
		sc.Input[Input::TimeFormat].SetCustomInputStrings("am / pm;24 Hour");
		sc.Input[Input::TimeFormat].SetCustomInputIndex(0);

		sc.Input[Input::ExpectedImpactHighColor].Name = "Expected Impact High Color";
		sc.Input[Input::ExpectedImpactHighColor].SetColor(RGB(252, 2, 2));

		sc.Input[Input::ExpectedImpactMediumColor].Name = "Expected Impact Medium Color";
		sc.Input[Input::ExpectedImpactMediumColor].SetColor(RGB(247, 152, 55));
		
		sc.Input[Input::ExpectedImpactLowColor].Name = "Expected Impact Low Color";
		sc.Input[Input::ExpectedImpactLowColor].SetColor(RGB(249, 227, 46));

		sc.Input[Input::ExpectedImpactNonEconomicColor].Name = "Expected Impact Non-Economic Color";
		sc.Input[Input::ExpectedImpactNonEconomicColor].SetColor(RGB(185, 186, 188));

		sc.Input[Input::EventTextColor].Name = "Event Text Color";
		sc.Input[Input::EventTextColor].SetColor(RGB(0, 0, 0));

		sc.Input[Input::EventTextBackgroundColor].Name = "Event Text Background Color";
		sc.Input[Input::EventTextBackgroundColor].SetColor(RGB(244, 246, 249));

		sc.Input[Input::RegionsVisible].Name = "Chart->Chart Settings->Region Number 1->Visible";
		sc.Input[Input::RegionsVisible].SetYesNo(1);

		sc.Input[Input::StrikeOutPastEvents].Name = "Strikeout Past Events";
		sc.Input[Input::StrikeOutPastEvents].SetYesNo(1);

		sc.Input[Input::PastEventsTextColor].Name = "Past Events Text Color";
		sc.Input[Input::PastEventsTextColor].SetColor(RGB(127, 127, 127));

		sc.Input[Input::ItalicUpcomingEvents].Name = "Italic Upcoming Events";
		sc.Input[Input::ItalicUpcomingEvents].SetYesNo(1);

		sc.Input[Input::UpcomingEventsTextColor].Name = "Upcoming Events Text Color";
		sc.Input[Input::UpcomingEventsTextColor].SetColor(RGB(0, 0, 0));

		sc.Input[Input::UpcomingEventsBackgroundColor].Name = "Upcoming Events Background Color";
		sc.Input[Input::UpcomingEventsBackgroundColor].SetColor(RGB(255, 255, 217));

		sc.Input[Input::HidePastEvents].Name = "Hide Past Events";
		sc.Input[Input::HidePastEvents].SetYesNo(0);

		sc.Input[Input::CurrentEventsTextColor].Name = "Current Events Text Color";
		sc.Input[Input::CurrentEventsTextColor].SetColor(RGB(0, 0, 0));

		sc.Input[Input::CurrentEventsBackgroundColor].Name = "Current Events Background Color";
		sc.Input[Input::CurrentEventsBackgroundColor].SetColor(RGB(0, 255, 0));

		sc.Input[Input::FFUpdateIntervalInMinutes].Name = "Forex Factory Event Update Interval (minutes)";
		sc.Input[Input::FFUpdateIntervalInMinutes].SetInt(9);
		// Logic - Don't need to continually blast FF with requests, so set to 5 minutes as lowest
		sc.Input[Input::FFUpdateIntervalInMinutes].SetIntLimits(5, 240);

		sc.Input[Input::BoldCurrentEvents].Name = "Bold Current Events";
		sc.Input[Input::BoldCurrentEvents].SetYesNo(1);

		sc.Input[Input::ShowTomorrowsEvents].Name = "Show Tomorrow's Events";
		sc.Input[Input::ShowTomorrowsEvents].SetYesNo(0);

		sc.Input[Input::TomorrowEventHeaderMarkerColor].Name = "Tomorrow's Event Header Marker Color";
		sc.Input[Input::TomorrowEventHeaderMarkerColor].SetColor(RGB(100, 100, 100));

		sc.Input[Input::TomorrowEventHeaderTextColor].Name = "Tomorrow's Event Header Text Color";
		sc.Input[Input::TomorrowEventHeaderTextColor].SetColor(RGB(255, 255, 255));

		sc.Input[Input::TomorrowEventHeaderBackgroundColor].Name = "Tomorrow's Event Header Background Color";
		sc.Input[Input::TomorrowEventHeaderBackgroundColor].SetColor(RGB(50, 50, 50));


#pragma endregion

#pragma region Subgraphs
		sc.Subgraph[Graph::UpcomingEventExpectedImpactHigh].Name = "Upcoming Event: Impact High";
		sc.Subgraph[Graph::UpcomingEventExpectedImpactHigh].DrawStyle = DRAWSTYLE_POINTHIGH;
		sc.Subgraph[Graph::UpcomingEventExpectedImpactHigh].PrimaryColor = sc.Input[Input::ExpectedImpactHighColor].GetColor();

		sc.Subgraph[Graph::UpcomingEventExpectedImpactMedium].Name = "Upcoming Event: Impact Medium ";
		sc.Subgraph[Graph::UpcomingEventExpectedImpactMedium].DrawStyle = DRAWSTYLE_POINTHIGH;
		sc.Subgraph[Graph::UpcomingEventExpectedImpactMedium].PrimaryColor = sc.Input[Input::ExpectedImpactMediumColor].GetColor();

		sc.Subgraph[Graph::UpcomingEventExpectedImpactLow].Name = "Upcoming Event: Impact Low";
		sc.Subgraph[Graph::UpcomingEventExpectedImpactLow].DrawStyle = DRAWSTYLE_POINTHIGH;
		sc.Subgraph[Graph::UpcomingEventExpectedImpactLow].PrimaryColor = sc.Input[Input::ExpectedImpactLowColor].GetColor();

		sc.Subgraph[Graph::UpcomingEventExpectedImpactNonEconomic].Name = "Upcoming Event: Impact Non-Economic";
		sc.Subgraph[Graph::UpcomingEventExpectedImpactNonEconomic].DrawStyle = DRAWSTYLE_POINTHIGH;
		sc.Subgraph[Graph::UpcomingEventExpectedImpactNonEconomic].PrimaryColor = sc.Input[Input::ExpectedImpactNonEconomicColor].GetColor();

		// TODO: Review naming semantics with EventHappeningRightNow and Current Event
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactHigh].Name = "Current Event: Impact High";
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactHigh].DrawStyle = DRAWSTYLE_COLORBARHOLLOW;
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactHigh].PrimaryColor = sc.Input[Input::ExpectedImpactHighColor].GetColor();

		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactMedium].Name = "Current Event: Impact Medium";
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactMedium].DrawStyle = DRAWSTYLE_COLORBARHOLLOW;
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactMedium].PrimaryColor = sc.Input[Input::ExpectedImpactMediumColor].GetColor();

		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactLow].Name = "Upcoming Event: Impact Low";
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactLow].DrawStyle = DRAWSTYLE_COLORBARHOLLOW;
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactLow].PrimaryColor = sc.Input[Input::ExpectedImpactLowColor].GetColor();

		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactNonEconomic].Name = "Upcoming Event: Impact Non-Economic";
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactNonEconomic].DrawStyle = DRAWSTYLE_COLORBARHOLLOW;
		sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactNonEconomic].PrimaryColor = sc.Input[Input::ExpectedImpactNonEconomicColor].GetColor();
#pragma endregion

		return;
	}

	// Force fill space but don't adjust smaller if it's already set larger than required.
	// Idea here would be to flag where future events occur on the chart timeline
	// May not really be needed at this point based on how upcoming events are now flagged
	// and subgraphs exist to alert on it
	if (sc.NumFillSpaceBars < unsigned(sc.Input[Input::FillSpace].GetInt()))
		sc.NumFillSpaceBars = sc.Input[Input::FillSpace].GetInt();

#pragma region Event Filters
#pragma region Currency Filter
	// Array of Currencies
	if (p_Currencies == NULL)
	{
		p_Currencies = new std::vector<SCString>;
		sc.SetPersistentPointer(p_Ptr::Currencies, p_Currencies);
	}
	else
		p_Currencies->clear();

	// This creates a filter for currencies based on user selection
	if (sc.Input[Input::CurrencyAUD].GetYesNo())
		p_Currencies->push_back("AUD");
	if (sc.Input[Input::CurrencyCAD].GetYesNo())
		p_Currencies->push_back("CAD");
	if (sc.Input[Input::CurrencyCHF].GetYesNo())
		p_Currencies->push_back("CHF");
	if (sc.Input[Input::CurrencyCNY].GetYesNo())
		p_Currencies->push_back("CNY");
	if (sc.Input[Input::CurrencyEUR].GetYesNo())
		p_Currencies->push_back("EUR");
	if (sc.Input[Input::CurrencyGBP].GetYesNo())
		p_Currencies->push_back("GBP");
	if (sc.Input[Input::CurrencyJPY].GetYesNo())
		p_Currencies->push_back("JPY");
	if (sc.Input[Input::CurrencyNZD].GetYesNo())
		p_Currencies->push_back("NZD");
	if (sc.Input[Input::CurrencyUSD].GetYesNo())
		p_Currencies->push_back("USD");
	if (sc.Input[Input::CurrencyAll].GetYesNo())
		p_Currencies->push_back("ALL");
#pragma endregion

#pragma region Impact Filter
	// Array of Impacts
	if (p_ExpectedImpact == NULL)
	{
		p_ExpectedImpact = new std::vector<SCString>;
		sc.SetPersistentPointer(p_Ptr::ExpectedImpact, p_ExpectedImpact);
	}
	else
		p_ExpectedImpact->clear();

	// This creates a filter for impact levels based on user selection
	if (sc.Input[Input::ExpectedImpactHigh].GetYesNo())
		p_ExpectedImpact->push_back("High");
	if (sc.Input[Input::ExpectedImpactMedium].GetYesNo())
		p_ExpectedImpact->push_back("Medium");
	if (sc.Input[Input::ExpectedImpactLow].GetYesNo())
		p_ExpectedImpact->push_back("Low");
	if (sc.Input[Input::ExpectedImpactNonEconomic].GetYesNo())
		p_ExpectedImpact->push_back("Holiday");
#pragma endregion
#pragma endregion

	// Array of FFEvent structs for each event
	if (p_FFEvents == NULL)
	{
		p_FFEvents = new std::vector<FFEvent>;
		sc.SetPersistentPointer(p_Ptr::FFEvents, p_FFEvents);
	}

	// Array of FFEventToDraw structs for each event
	// TODO: Fix naming plural inconsistency
	if (p_FFEventsToDraw == NULL)
	{
		p_FFEventsToDraw = new std::vector<FFEventToDraw>;
		sc.SetPersistentPointer(p_Ptr::FFEventsToDraw, p_FFEventsToDraw);
	}

#pragma region HTTP Fetch Forex Events
	// Here we only focus on fetching the weeks FF events based on impact/currency
	// Could also filter on today/tomorrow but this data isn't refreshed that often, thus it makes
	// sense to store it all and 'roll' through it in the processing loop later on
	// At least it makes sense for now :)
	// HTTP Request Start
	// Sierra Reference:
	// https://www.sierrachart.com/index.php?page=doc/ACSIL_Members_Functions.html#scMakeHTTPRequest
	enum {REQUEST_NOT_SENT = 0, REQUEST_SENT, REQUEST_RECEIVED}; // status codes
	int &RequestState = sc.GetPersistentInt(p_Int::RequestState); // latest request status

	// Only run on full recalc or if update interval has passed
	if (
		sc.Index == 0 ||
		CurrentDateTime.GetTimeInSeconds() > (LastFFUpdate.GetTimeInSeconds() + (60 * sc.Input[Input::FFUpdateIntervalInMinutes].GetInt()) + RandomSecond)
		)
	{
		if (RequestState == REQUEST_NOT_SENT)
		{
			if (!sc.MakeHTTPRequest("https://nfs.faireconomy.media/ff_calendar_thisweek.json"))
			{
				sc.AddMessageToLog("Forex Factory Events: Error Making HTTP Request", 1);
				RequestState = REQUEST_NOT_SENT;
			}
			else
				RequestState = REQUEST_SENT;
		}
	}

	if (RequestState == REQUEST_SENT && sc.HTTPResponse != "")
	{
		// If re-loading data, then clear if previous data exists...
		if (p_FFEvents != NULL)
			p_FFEvents->clear();

		RequestState = REQUEST_RECEIVED;
		HttpResponseContent = sc.HTTPResponse;
		sc.AddMessageToLog(sc.HTTPResponse, 0);
		std::string JsonEventsString = sc.HTTPResponse;
		auto j = json::parse(JsonEventsString);
		for (auto& elem : j)
		{
			std::string Title = elem["title"];
			std::string Country = elem["country"];
			std::string Date = elem["date"];
			std::string Impact = elem["impact"];
			std::string Forecast = elem["forecast"];
			std::string Previous = elem["previous"];

			// For the date, SC doesn't have a native parser I'm aware of, so we use sscanf to pull out the values
			// individually for year, month, day, etc...
			// Not currently using utc offset for anything
			// Example Format: 2022-05-22T19:01:00-04:00
			// https://stackoverflow.com/questions/26895428/how-do-i-parse-an-iso-8601-date-with-optional-milliseconds-to-a-struct-tm-in-c
			int Year, Month, Day, Hour, Minute, Second, UTCOffsetHour, UTCOffsetMinute = 0;
			sscanf(Date.c_str(), "%d-%d-%dT%d:%d:%d-%d:%d", &Year, &Month, &Day, &Hour, &Minute, &Second, &UTCOffsetHour, &UTCOffsetMinute);

			// Now after we have the year, month, day etc... extracted, we load them into an SCDateTime variable
			// https://www.sierrachart.com/index.php?page=doc/ACSIL_Members_Functions.html#scDateTimeToString
			// https://www.sierrachart.com/SupportBoard.php?ThreadID=18682
			SCDateTime FFEventDateTime;
			FFEventDateTime.SetDateTimeYMDHMS(Year, Month, Day, Hour, Minute, Second);

			// Convert time to chart time zone
			// Assumption is chart time zone is set to local time zone
			// May need to look into this further. When doing compares for events
			// the event time needs to be converted to system time (ie, chart time)
			// But if a chart is a one off and set to alternate time zone then will be an issue
			// as alerts are compared against system time
			// FF Events are using New York Time
			SCDateTime EventDateTime = sc.ConvertDateTimeToChartTimeZone(FFEventDateTime, TIMEZONE_NEW_YORK);
			int EventDay = EventDateTime.GetDay();
			int EventHour = EventDateTime.GetHour();
			int EventMinute = EventDateTime.GetMinute();

			// Initial pre-filtering based on what user has selected for Currency and Impact Levels
			// Skip events that don't match currency filter
			auto it = find(p_Currencies->begin(), p_Currencies->end(), Country.c_str()) != p_Currencies->end();
			if (!it)
				continue;

			// Skip events that don't match impact filter
			it = find(p_ExpectedImpact->begin(), p_ExpectedImpact->end(), Impact.c_str()) != p_ExpectedImpact->end();
			if (!it)
				continue;

			// Now we have the data we need but in std::string type, but our struct is SCString
			// Need to convertusing c_str(), Is there a better way?
			// Convert std::string to SCString
			// https://www.sierrachart.com/SupportBoard.php?ThreadID=55783
			// https://www.cplusplus.com/reference/string/string/c_str/
			FFEvent TmpFFEvent = {
				Title.c_str(),
				Country.c_str(),
				EventDateTime,
				Impact.c_str(),
				Forecast.c_str(),
				Previous.c_str(),
			};

			// Finally, add the FFEvent to the end of the array of events
			p_FFEvents->insert(p_FFEvents->end(), TmpFFEvent);
		}
		// We FF events, now update FFUpdateInterval
		LastFFUpdate = GetNow(sc);
	}
	else if (RequestState == REQUEST_SENT && sc.HTTPResponse == "")
		return;

	// Reset state for next run
	RequestState = REQUEST_NOT_SENT;
#pragma endregion

	// Subgraphs Inital Values
	sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactHigh][sc.Index] = 0.0f;
	sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactMedium][sc.Index] = 0.0f;
	sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactLow][sc.Index] = 0.0f;
	sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactNonEconomic][sc.Index] = 0.0f;
	sc.Subgraph[Graph::UpcomingEventExpectedImpactHigh][sc.Index] = 0.0f;
	sc.Subgraph[Graph::UpcomingEventExpectedImpactMedium][sc.Index] = 0.0f;
	sc.Subgraph[Graph::UpcomingEventExpectedImpactLow][sc.Index] = 0.0f;
	sc.Subgraph[Graph::UpcomingEventExpectedImpactNonEconomic][sc.Index] = 0.0f;

#pragma region Process Forex Events
	// Here we focus on looping through fetched events and updating fonts/colors/etc...
	// based on user selection and as time passes. Since events were pre-filtered, these
	// are all the vents we want to Draw
	// Events are then pushed to the FFEventsToDraw for later drawing
	if (p_FFEventsToDraw != NULL)
		p_FFEventsToDraw->clear(); // Start with cleared state

	bool b_AddTomorrowHeader = true;
	// Loop through raw FF Events
	for (int i = 0; i < p_FFEvents->size(); i++)
	{
		// Copy of event to use. See below notes regarding direct reference...
		// TODO: Try this again without making a copy first...
		FFEvent Event = p_FFEvents->at(i);

		// Tmp event to build events to push to EventsToDraw
		FFEventToDraw TmpFFEventToDraw;

		// Set the H/M/S from Event
		// Note! This event DateTime has also been converted from NY Time to Chart Time in initial data load
		// For some reason Sierra Complains if you try to use the Event.date.GetDay() directly in the compare statements
		// TODO: Need to test again and see if still the case
		int EventDay = Event.date.GetDay();
		int EventHour = Event.date.GetHour();
		int EventMinute = Event.date.GetMinute();

		// Setup some logic for later use and easier readability
		bool b_EventIsToday = EventDay == CurrentDay;
		bool b_EventIsTomorrow = EventDay == CurrentDay + 1;
		bool b_EventIsTodayOrTomorrow = b_EventIsToday || b_EventIsTomorrow;
		
		// Skip event if it's not for today or tomorrow		
		if (!b_EventIsTodayOrTomorrow)
			continue;

		// If this is an event for tommorow but we don't want to show those, skip over it
		// Otherwise, if event for tomorrow, add the tomorrow header and update the flag to skip next go around
		if (b_EventIsTomorrow && !sc.Input[Input::ShowTomorrowsEvents].GetYesNo())
			continue;
		else if (b_EventIsTomorrow && b_AddTomorrowHeader)
		{
			FFEventToDraw TomorrowHeader = {
				"--- Tomorrow's Events ---",
				sc.Input[Input::TomorrowEventHeaderMarkerColor].GetColor(),
				sc.Input[Input::TomorrowEventHeaderTextColor].GetColor(),
				sc.Input[Input::TomorrowEventHeaderBackgroundColor].GetColor(),
				400,
				false,
				true
			};
			p_FFEventsToDraw->insert(p_FFEventsToDraw->end(), TomorrowHeader);
			b_AddTomorrowHeader = false; // Reset flag
		}

		// Set Default Text Colors
		TmpFFEventToDraw.EventTextColor = sc.Input[Input::EventTextColor].GetColor();
		TmpFFEventToDraw.EventTextBackgroundColor = sc.Input[Input::EventTextBackgroundColor].GetColor();

		// Set MarkerColor (Brush) based on High, Medium, Low, Holiday
		COLORREF MarkerColor = RGB(255, 255, 255);
		if (Event.impact == "High")
			TmpFFEventToDraw.MarkerColor = sc.Input[Input::ExpectedImpactHighColor].GetColor();
		else if (Event.impact == "Medium")
			TmpFFEventToDraw.MarkerColor = sc.Input[Input::ExpectedImpactMediumColor].GetColor();
		else if (Event.impact == "Low")
			TmpFFEventToDraw.MarkerColor = sc.Input[Input::ExpectedImpactLowColor].GetColor();
		else if (Event.impact == "Holiday")
			TmpFFEventToDraw.MarkerColor = sc.Input[Input::ExpectedImpactNonEconomicColor].GetColor();

		// Construct Event Text and display time in 24hr vs am/pm based on user input
		SCString EventText;
		if (sc.Input[Input::TimeFormat].GetInt())
			EventText.Format("%02d:%02d [%s] %s", EventHour, EventMinute, Event.country.GetChars(), Event.title.GetChars());
		else
		{
			if (EventHour == 12)
				EventText.Format("%2d:%02dpm [%s] %s", EventHour, EventMinute, Event.country.GetChars(), Event.title.GetChars());
			else if (EventHour > 12)
				EventText.Format("%2d:%02dpm [%s] %s", EventHour -= 12, EventMinute, Event.country.GetChars(), Event.title.GetChars());
			else
				EventText.Format("%2d:%02dam [%s] %s", EventHour, EventMinute, Event.country.GetChars(), Event.title.GetChars());
		}
		// Update event text
		TmpFFEventToDraw.EventText = EventText;
		
		// TODO: Need to fix this as we are changing these above based on time format
		// For now just reset them as we use to compare current time to event time
		EventDay = Event.date.GetDay();
		EventHour = Event.date.GetHour();
		EventMinute = Event.date.GetMinute();

		// Is this event happening right now based on hour/minute
		// For now it allows the current event to remain in 'Now' status for a minute
		// TODO: Decide if it makes sense to allow user to change this setting to different value
		bool EventHappeningRightNow = false; // Start off as false. Could probably key off the p_EventHappening instead...?
		if (EventHour == CurrentHour && EventMinute == CurrentMinute && !b_EventIsTomorrow)
		{
			// Update Subgraphs
			if (Event.impact == "High")
				sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactHigh][sc.Index] = 1.0f;
			else if (Event.impact == "Medium")
				sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactMedium][sc.Index] = 1.0f;
			else if (Event.impact == "Low")
				sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactLow][sc.Index] = 1.0f;
			else
				sc.Subgraph[Graph::EventHappeningRightNowExpectedImpactNonEconomic][sc.Index] = 1.0f;

			// Update Draw data
			if (sc.Input[Input::BoldCurrentEvents].GetYesNo())
				TmpFFEventToDraw.FontWeight = 700;
			TmpFFEventToDraw.EventTextColor = sc.Input[Input::CurrentEventsTextColor].GetColor();
			TmpFFEventToDraw.EventTextBackgroundColor = sc.Input[Input::CurrentEventsBackgroundColor].GetColor();
			EventHappeningRightNow = true;
		}

		// Past Events
		bool b_PastEvent = false;
		if (Event.date < CurrentDateTime && !EventHappeningRightNow)
		{
			TmpFFEventToDraw.EventTextColor = sc.Input[Input::PastEventsTextColor].GetColor();
			if (sc.Input[Input::StrikeOutPastEvents].GetYesNo())
				TmpFFEventToDraw.StrikeOut = true;
			b_PastEvent = true; // Flag this event as a past event
		}

		// Upcoming Events
		// For now, if event is within 10 minutes coming up
		// TODO: Decide if it makes sense to add input to see how far in advance a user wants this to trigger
		if (Event.date > CurrentDateTime && Event.date.GetTimeInSeconds() < CurrentDateTime.GetTimeInSeconds() + 60 * 10 && !b_EventIsTomorrow)
		{
			if (Event.impact == "High")
				sc.Subgraph[Graph::UpcomingEventExpectedImpactHigh][sc.Index] = 1.0f;
			else if (Event.impact == "Medium")
				sc.Subgraph[Graph::UpcomingEventExpectedImpactMedium][sc.Index] = 1.0f;
			else if (Event.impact == "Low")
				sc.Subgraph[Graph::UpcomingEventExpectedImpactLow][sc.Index] = 1.0f;
			else
				sc.Subgraph[Graph::UpcomingEventExpectedImpactNonEconomic][sc.Index] = 1.0f;

			if (sc.Input[Input::ItalicUpcomingEvents].GetYesNo())
				TmpFFEventToDraw.Italic = true;

			TmpFFEventToDraw.EventTextColor = sc.Input[Input::UpcomingEventsTextColor].GetColor();
			TmpFFEventToDraw.EventTextBackgroundColor = sc.Input[Input::UpcomingEventsBackgroundColor].GetColor();
		}

		if (sc.Input[Input::HidePastEvents].GetYesNo() && b_PastEvent)
			continue; // Skip loop and don't add event to draw as it's a past event

		// If here, add event to draw later
		p_FFEventsToDraw->insert(p_FFEventsToDraw->end(), TmpFFEventToDraw);

	}
#pragma endregion

	// Draw Events with GDI
	// TODO: Think if we need to call this all the time or not? Same with subgraphs...
	// If time based maybe not, but if tick based then probably so... but for now...
	// Could probably get the type of chart and adjust updates according to time vs tick etc...
	sc.p_GDIFunction = DrawToChart;
}

void DrawToChart(HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc)
{
	// Pointer to Struct Array for FF Events to Draw
	std::vector<FFEventToDraw>* p_FFEventsToDraw = reinterpret_cast<std::vector<FFEventToDraw>*>(sc.GetPersistentPointer(p_Ptr::FFEventsToDraw));

	// By default Region 1 is visible but no way to access this via ACSIL
	// Assume it's enabled and offset so we don't draw under it
	int EventSpacing = sc.Input[Input::VerticalOffset].GetInt();
	if (sc.Input[Input::RegionsVisible].GetYesNo())
		EventSpacing += 22;

	// Loop through and process events to draw
	for (int i = 0; i < p_FFEventsToDraw->size(); i++)
	{	
		// Increment spacing only after first event
		if (i > 0)
			EventSpacing += sc.Input[Input::FontSize].GetInt() + sc.Input[Input::EventSpacingOffset].GetInt();

		// Draw Marker (Rectangle) to the Left of the Event Text
		HBRUSH Brush = CreateSolidBrush(p_FFEventsToDraw->at(i).MarkerColor);
		// Select the brush into the device context
		HGDIOBJ PriorBrush = SelectObject(DeviceContext, Brush);
		// Draw a rectangle next to event
		Rectangle(DeviceContext,
			sc.StudyRegionLeftCoordinate + sc.Input[Input::HorizontalOffset].GetInt(), // Left
			sc.StudyRegionTopCoordinate + EventSpacing, // Top
			sc.StudyRegionLeftCoordinate + sc.Input[Input::HorizontalOffset].GetInt() + 20, // Right
			sc.StudyRegionTopCoordinate + sc.Input[Input::FontSize].GetInt() + EventSpacing // Bottom
		);

		// Remove the brush from the device context and put the prior brush back in. This is critical!
		SelectObject(DeviceContext, PriorBrush);
		// Delete the brush.  This is critical!  If you do not do this, you will end up with
		// a GDI leak and crash Sierra Chart.
		DeleteObject(Brush);

		// Create font
		HFONT hFont;
		hFont = CreateFont(
			sc.Input[Input::FontSize].GetInt(), // Font size from user input
			0,
			0,
			0,
			p_FFEventsToDraw->at(i).FontWeight, // Weight
			p_FFEventsToDraw->at(i).Italic, // Italic
			FALSE, // Underline
			p_FFEventsToDraw->at(i).StrikeOut, // StrikeOut
			DEFAULT_CHARSET,
			OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH,
			TEXT(sc.ChartTextFont()) // Pulls font used in chart
		);
		SelectObject(DeviceContext, hFont);

		// Set text/colors
		::SetTextAlign(DeviceContext, TA_NOUPDATECP);
		::SetTextColor(DeviceContext, p_FFEventsToDraw->at(i).EventTextColor);
		::SetBkColor(DeviceContext, p_FFEventsToDraw->at(i).EventTextBackgroundColor);

		::TextOut(
			DeviceContext,
			sc.StudyRegionLeftCoordinate + sc.Input[Input::HorizontalOffset].GetInt() + 23,
			sc.StudyRegionTopCoordinate + EventSpacing,
			p_FFEventsToDraw->at(i).EventText,
			p_FFEventsToDraw->at(i).EventText.GetLength()
		);
		DeleteObject(hFont);
	}
	return;
}