{***************************************************}
{                                                   }
{   Windows 3.1 DDEML Demonstration Program         }
{   Copyright (c) 1992 by Borland International     }
{                                                   }
{***************************************************}

program DDEMLServer;

{ This sample application uses the DDEML library in the server side of a
  cooperative application.  This server is a simple data-entry application
  which allows an operator to enter three data items, which are made
  available through DDE to interested clients.

  This server makes its service available under the following names:

       Service: 'DataEntry'
       Topic  : 'SampledData'
       Items  : 'DataItem1', 'DataItem2', 'DataItem3'

  Conceivably, other topics under this service could be defined.  Things
  such as historical data, information about the sampling, and so on
  might make useful topics.

  You must run this server BEFORE running the client (DDEMLCLI.PAS), or
  the client will fail the connection.

  The interface to this server is defined by the list of names (Service,
  Topic, and Items) in the separate unit called DataEntry (DATAENTR.TPU).
  The server makes the Items available in cf_Text format; they can be
  converted and stored locally as integers by the client.
}

uses Strings, WinTypes, WinProcs, OWindows, ODialogs, Win31, DDEML,
  ShellAPI, BWCC, DataEntry;

{$R DDEMLSRV}

const

{ Resource IDs }

  id_Menu    = 100;
  id_About   = 100;
  id_Icon    = 100;

  id_Value1  = 401;  { Used with the DataEntry Dialog }
  id_Value2  = 402;
  id_Value3  = 403;

  st_Message =   1;

{ Menu command IDs }

  cm_DataEnter = 201;
  cm_DataClear = 202;
  cm_HelpAbout = 300;

type

{ Application main window }

  PDDEServerWindow = ^TDDEServerWindow;
  TDDEServerWindow = object(TWindow)
    Inst       : Longint;
    CallBack   : TCallback;
    ServiceHSz : HSz;
    TopicHSz   : HSz;
    ItemHSz    : array [1..NumValues] of HSz;
    ConvHdl    : HConv;
    Advising   : array [1..NumValues] of Boolean;

    DataSample : TDataSample;

    constructor Init(AParent: PWindowsObject; ATitle: PChar);
    destructor  Done; virtual;

    procedure GetWindowClass(var AWndClass: TWndClass); virtual;
    function  GetClassName: PChar; virtual;
    procedure SetupWindow; virtual;
    procedure Paint(PaintDC: HDC; var PaintInfo: TPaintStruct); virtual;

    procedure CMDataEnter(var Msg: TMessage);
      virtual cm_First + cm_DataEnter;
    procedure CMDataClear(var Msg: TMessage);
      virtual cm_First + cm_DataClear;
    procedure CMHelpAbout(var Msg: TMessage);
      virtual cm_First + cm_HelpAbout;

    function  MatchTopicAndService(Topic, Service: HSz): Boolean; virtual;
    function  MatchTopicAndItem(Topic, Item: HSz): Integer; virtual;
    function  WildConnect(Topic, Service: HSz;
      ClipFmt: Word): HDDEData; virtual;
    function  AcceptPoke(Item: HSz; ClipFmt: Word;
      Data: HDDEData): Boolean; virtual;
    function  DataRequested(TransType: Word; ItemNum: Integer;
      ClipFmt: Word): HDDEData; virtual;
  end;


{ Application object }

  TDDEServerApp = object(TApplication)
    procedure InitMainWindow; virtual;
  end;


{ Initialized globals }

const
  DemoTitle   : PChar = 'DDEML Demo, Server Application';

  MaxAdvisories = 100;
  NumAdvLoops : Integer = 0;


{ Global variables }

var
  App: TDDEServerApp;


{ Local Function: CallBack Procedure for DDEML }

{ This callback procedure responds to all transactions generated by the
  DDEML.  The target Window object is obtained from the stored global,
  and the appropriate methods within that objects are used to respond
  to the given transaction, as indicated by the CallType parameter.
}
function CallbackProc(CallType, Fmt: Word; Conv: HConv; HSz1, HSz2: HSZ;
  Data: HDDEData; Data1, Data2: Longint): HDDEData; export;
var
  ThisWindow: PDDEServerWindow;
  ItemNum   : Integer;
begin
  CallbackProc := 0;    { See if proved otherwise }

  ThisWindow := PDDEServerWindow(App.MainWindow);

  case CallType of

    xtyp_WildConnect:
      CallbackProc := ThisWindow^.WildConnect(HSz1, HSz2, Fmt);

    xtyp_Connect:
      if Conv = 0 then
      begin
        if ThisWindow^.MatchTopicAndService(HSz1, HSz2) then
          CallbackProc := 1;   { Connected! }
      end;
{ When a connection is confirmed, record the conversation handle as the
  window's own.
}
    xtyp_Connect_Confirm:
      ThisWindow^.ConvHdl := Conv;

{ The client has requested data, either as a direct request or
  in response to an advisory.  Return the current state of the
  data.
}
    xtyp_AdvReq, xtyp_Request:
      begin
        ItemNum := ThisWindow^.MatchTopicAndItem(HSz1, HSz2);
        if ItemNum > 0 then
          CallbackProc := ThisWindow^.DataRequested(CallType, ItemNum, Fmt);
      end;

{ Respond to Poke requests ... this demo only allows Pokes of DataItem1.
  Return dde_FAck to acknowledge the receipt, 0 otherwise.
}
    xtyp_Poke:
      begin
        if ThisWindow^.AcceptPoke(HSz2, Fmt, Data) then
          CallbackProc := dde_FAck;
      end;

{ The client has requested the start of an advisory loop.  Note
  that we assume a "hot" loop.  Set the Advising flag to indicate
  the open loop, which will be checked whenever the data is changed.
}
    xtyp_AdvStart:
      begin
        ItemNum := ThisWindow^.MatchTopicAndItem(HSz1, HSz2);
        if ItemNum > 0 then
        begin
          if NumAdvLoops < MaxAdvisories then    { Arbitrary number }
          begin
            Inc(NumAdvLoops);
            ThisWindow^.Advising[ItemNum] := True;
            CallbackProc := 1;
          end;
        end;
      end;

{ The client has requested the advisory loop to terminate.
}
    xtyp_AdvStop:
      begin
        ItemNum := ThisWindow^.MatchTopicAndItem(HSz1, HSz2);
        if ItemNum > 0 then
        begin
          if NumAdvLoops > 0 then
          begin
            Dec(NumAdvLoops);
            if NumAdvLoops = 0 then
              ThisWindow^.Advising[ItemNum] := False;
            CallbackProc := 1;
          end;
        end;
      end;
  end;  { Case CallType }

end;


{ TDDEServerWindow Methods }

{ Constructs an instance of the DDE Server Window.  Calls on the
  inherited constructor, then sets up this objects own instandce
  data.
}
constructor TDDEServerWindow.Init(AParent: PWindowsObject; ATitle: PChar);
var
  I : Integer;
begin
  TWindow.Init(AParent, ATitle);

  Inst      := 0;      { Must be zero for first call to DdeInitialize }
  @CallBack := nil;    { MakeProcInstance is called in SetupWindow    }

  for I := 1 to NumValues do
  begin
    DataSample[I]:= 0;
    Advising[I]  := False;
  end;
end;

{ Destroys an instance of the DDE Server Window.  Checks to see if the
  Callback Proc Instance had been created, and frees it if so.  Also 
  calls DdeUninitialize to terminate the conversation.  Then just calls
  on the ancestral destructor to finish.
}
destructor TDDEServerWindow.Done;
var
  I : Integer;
begin
  if ServiceHSz <> 0 then
    DdeFreeStringHandle(Inst, ServiceHSz);
  if TopicHSz <> 0 then
    DdeFreeStringHandle(Inst, TopicHSz);
  for I := 1 to NumValues do
    if ItemHSz[I] <> 0 then
      DdeFreeStringHandle(Inst, ItemHSz[I]);

  if Inst <> 0 then
    DdeUninitialize(Inst);   { Ignore the return value }
    
  if @CallBack <> nil then
    FreeProcInstance(@CallBack);

  TWindow.Done;
end;

{ Redefines GetWindowClass to give this application its own Icon and
  default menu.
}
procedure TDDEServerWindow.GetWindowClass(var AWndClass: TWndClass);
begin
  TWindow.GetWindowClass(AWndClass);
  AWndClass.hIcon := LoadIcon(AWndClass.hInstance, PChar(id_Icon));
  AWndClass.lpszMenuName := PChar(id_Menu);
end;

{ Returns the class name of this window.  This is necessary since we
  redefine the inherited GetWindowClass method, above.
}
function TDDEServerWindow.GetClassName: PChar;
begin
  GetClassName := 'TDDEServerWindow';
end;

{ Completes the initialization of the DDE Server Window.  Initializes
  the use of the DDEML by registering the services provided by this
  application.  Recall that the actual names used to register are
  defined in a separate unit (DataEntry), so that they can be used
  by the client as well.
}
procedure TDDEServerWindow.SetupWindow;
var
  I : Integer;
begin
  TWindow.SetupWindow;

  @CallBack:= MakeProcInstance(@CallBackProc, HInstance);

  if DdeInitialize(Inst, CallBack, 0, 0) = dmlErr_No_Error then
  begin
    ServiceHSz:= DdeCreateStringHandle(Inst, DataEntryName, cp_WinAnsi);
    TopicHSz  := DdeCreateStringHandle(Inst, DataTopicName, cp_WinAnsi);
    for I := 1 to NumValues do
      ItemHSz[I] := DdeCreateStringHandle(Inst, DataItemNames[I],
        cp_WinAnsi);

    if DdeNameService(Inst, ServiceHSz, 0, dns_Register) = 0 then
    begin
      MessageBox(HWindow, 'Registration failed.', Application^.Name,
        mb_IconStop);
      PostQuitMessage(0);
    end;
  end
  else
    PostQuitMessage(0);
end;

procedure TDDEServerWindow.Paint(PaintDC: HDC; var PaintInfo: TPaintStruct);
type
  TDataItem = record
    Name: Pointer;
    Value: Integer;
  end;
  TData = array[1..NumValues] of TDataItem;
var
  R: TRect;
  S: array[0..255] of Char;
  S1: array[0..512] of Char;
  Len, I: Integer;
  Data: TData;
begin
  GetClientRect(HWindow, R);
  InflateRect(R, -10, 0);
  LoadString(hInstance, st_Message, S, SizeOf(S));
  for I := 1 to NumValues do
  begin
    Data[I].Name := DataItemNames[I];
    Data[I].Value := DataSample[I];
  end;
  Len := wvsPrintf(S1, S, Data);
  DrawText(PaintDC, S1, Len, R, dt_WordBreak);
end;

{ Returns True if the given Topic and Service match those supported
  by this application.  False otherwise.
}
function TDDEServerWindow.MatchTopicAndService(Topic, Service: HSz): Boolean;
begin
  MatchTopicAndService := False;
  if DdeCmpStringHandles(TopicHSz, Topic) = 0 then
    if DdeCmpStringHandles(ServiceHSz, Service) = 0 then
      MatchTopicAndService := True;
end;

{ Determines if the given Topic and Item match one supported by this
  application.  Returns the Item Number of the supported item (in the
  range 1..NumValues) if one is found, and zero if no match.
}
function TDDEServerWindow.MatchTopicAndItem(Topic, Item: HSz): Integer;
var
  I : Integer;
begin
  MatchTopicAndItem := 0;
  if DdeCmpStringHandles(TopicHSz, Topic) = 0 then
    for I := 1 to NumValues do
      if DdeCmpStringHandles(ItemHSz[I], Item) = 0 then
        MatchTopicAndItem := I;
end;

{ Responds to wildcard connect requests.  These requests are generated
  whenever a client tries to connect to a server with either service or topic
  name set to 0.  If a server detects a wild card match, it returns a
  handle to an array of THSZPair's containing the matching supported Service
  and Topic.
}
function TDDEServerWindow.WildConnect(Topic, Service: HSz;
  ClipFmt: Word): HDDEData;
var
  TempPairs: array [0..1] of THSZPair;
  Matched  : Boolean;
begin
  TempPairs[0].hszSvc  := ServiceHSz;
  TempPairs[0].hszTopic:= TopicHSz;
  TempPairs[1].hszSvc  := 0;     { 0-terminate the list }
  TempPairs[1].hszTopic:= 0;

  Matched := False;

  if (Topic= 0) and (Service = 0) then
    Matched := True                    { Complete wildcard }
  else
    if (Topic = 0) and (DdeCmpStringHandles(Service, ServiceHSz) = 0) then
      Matched := True
    else
      if (DdeCmpStringHandles(Topic, TopicHSz) = 0) and (Service = 0) then
        Matched := True;

  if Matched then
    WildConnect := DdeCreateDataHandle(Inst, @TempPairs, SizeOf(TempPairs),
      0, 0, ClipFmt, 0)
  else
    WildConnect := 0;
end;

{ Accepts and acts upon Poke requests from the Client.  For this
  demonstration, allows only the value of DataItem1 to be changed by a Poke.
}
function TDDEServerWindow.AcceptPoke(Item: HSz; ClipFmt: Word;
  Data: HDDEData): Boolean;
var
  DataStr   : TDataString;
  Err       : Integer;
  TempSample: TDataSample;
begin
  if (DdeCmpStringHandles(Item, ItemHSz[1]) = 0) and (ClipFmt = cf_Text) then
  begin
    DdeGetData(Data, @DataStr, SizeOf(DataStr), 0);
    Val(DataStr, TempSample[1], Err);

    if TempSample[1] <> DataSample[1] then
    begin
      DataSample[1] := TempSample[1];
      if Advising[1] then
        DdePostAdvise(Inst, TopicHSz, ItemHSz[1]);
    end;
    InvalidateRect(HWindow, nil, True);
    AcceptPoke := True;
  end
  else
    AcceptPoke := False;
end;

{ Returns the data requested by the given TransType and ClipFmt values.
  This could happen either in response to either an xtyp_Request or an
  xtyp_AdvReq.  The ItemNum parameter indicates which of the supported
  items (in the range 1..NumValues) was requested (note that this method
  assumes that the caller has already established validity and ID of the
  requested item using MatchTopicAndItem).  The corresponding data from
  the DataSample instance variable is converted to text and returned.
}
function TDDEServerWindow.DataRequested(TransType: Word; ItemNum: Integer;
  ClipFmt: Word): HDDEData;
var
  ItemStr: TDataString;   { Defined in DataEntry.TPU }
begin
  if ClipFmt = cf_Text then
  begin
    Str(DataSample[ItemNum], ItemStr);
    DataRequested := DdeCreateDataHandle(Inst, @ItemStr, StrLen(ItemStr) + 1,
      0, ItemHSz[ItemNum], ClipFmt, 0);
  end
  else
    DataRequested := 0;
end;

{ Activates the data-entry dialog, and updates the stored
  data when complete.
}
procedure TDDEServerWindow.CMDataEnter(var Msg: TMessage);
const
  ValEditIds : array [1..NumValues] of Integer = (id_Value1,
    id_Value2, id_Value3);
var
  DataEntry  : PDialog;
  Err, I     : Integer;
  TempSample : TDataSample;
  Ed         : PEdit;
  TransferRec: array [1..NumValues] of record
                                         ValStr : array [0..19] of Char;
                                       end;
begin
  DataEntry := New(PDialog, Init(@Self, 'DATAENTRY'));

  for I := 1 to NumValues do
  begin
    Str(DataSample[I], TransferRec[I].ValStr);
    New(Ed, InitResource(DataEntry, ValEditIds[I],
      SizeOf(TransferRec[I].ValStr)));
  end;

  DataEntry^.TransferBuffer := @TransferRec;

  if Application^.ExecDialog(DataEntry) = IdOK then
  begin
    for I := 1 to NumValues do
    begin
      Val(TransferRec[I].ValStr, TempSample[I], Err);

      if TempSample[I] <> DataSample[I] then
      begin
        DataSample[I] := TempSample[I];
        if Advising[I] then
          DdePostAdvise(Inst, TopicHSz, ItemHSz[I]);
      end;
    end;
    InvalidateRect(HWindow, nil, True);
  end;
end;

{ Clears the current data.
}
procedure TDDEServerWindow.CMDataClear(var Msg: TMessage);
var
  I : Integer;
begin
  for I := 1 to NumValues do
  begin
    DataSample[I] := 0;
    if Advising[I] then
      DdePostAdvise(Inst, TopicHSz, ItemHSz[I]);
  end;
  InvalidateRect(HWindow, nil, True);
end;

{ Posts the about box dialog for the DDE Server.
}
procedure TDDEServerWindow.CMHelpAbout(var Msg: TMessage);
begin
  Application^.ExecDialog(New(PDialog, Init(@Self, PChar(id_About))));
end;


{ TDDEServerApp Methods }

procedure TDDEServerApp.InitMainWindow;
begin
  MainWindow := New(PDDEServerWindow, Init(nil, Application^.Name));
end;


{ Main program }

begin
  App.Init(DemoTitle);
  App.Run;
  App.Done;
end.

