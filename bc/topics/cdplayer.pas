PROGRAM CD_Player;
USES Crt,Dos;

CONST
  ctIn=3;   { Yeu cau ve nhan thong tin }
TYPE
  Request=RECORD
            ReqLen   : Byte;
            SuCode   : Byte;
            Command  : Byte; { Ma lenh yeu cau }
            Status   : Word; { Ket qua va tinh trang }
            Reserved : ARRAY[1..8] OF Byte
          END;

  { Ket qua nhap xuat va cac dia chi }
  IOControl=RECORD
              Rh    : Request;
              Media : Byte;
              Addr  : Pointer; { Dia chi vung diem tiep theo }
              NByte : Word;    { Do lon cua Address tro den }
              Start : Word;
              PtrID : Pointer
            END;

  { Bo dem thoi gian va dung luong dia CDROM }
  Counter=RECORD
            Frame  : Byte;
            Sec    : Byte;
            Min    : Byte;
            Sector : LongInt
          END;

     { Lay thong tin ve dia hien thoi }
     DiskInfo=RECORD
                LoTrack : Byte;   { Track nho nhat }
                HiTrack : Byte;   { Track lon nhat }
                LeadOut : Counter { Thoi luong cua dia }
              END;

     { Lay thong tin ve Track hien thoi }
     TrackInfo=RECORD
                 Track   : Byte;
                 Start   : Counter;
                 Sectors : LongInt;
               END;

VAR
  CDInfo     : DiskInfo; { Thong tin ve dia }
  DanhSach   : STRING;   { Danh sach cac o dia CDROM }
  HiT,LoT    : Byte;     { Byte co nhat va Byte thap nhat }
  Info       : TrackInfo;{ Thong tin ve moi Track }
  I, DiskNum : Byte;     { So hieu o dia }
  Busy       : Boolean;  { =TRUE neu o dia dang choi nhac}
  Ready      : Boolean;  { =TRUE neu o dia da thuc hien yeu cau}
  Error      : Boolean;  { =TRUE neu xay ra loi}

{ Ham lay thong tin ban dau cua dia }
FUNCTION GetCDLetters : STRING;
VAR
  Regs    : Registers;
  Buf     : ARRAY[1..26] OF Byte;
  Lett    : STRING;
  I       : Byte;
  CDCount : Byte;

 BEGIN
   GetCDLetters:='';
   Regs.AX:=$1500;
   Regs.BX:=1; { Chi co mot o dia CDROM }
   Regs.CX:=4; { So hieu o dia CDROM A=0, B=1, ..., E=4 }
   Intr($2F,Regs);
   CDCount:=Regs.BX;
   IF CDCount=0 THEN EXIT;
   Regs.AX:=$150D;
   Regs.ES:=Seg(Buf);
   Regs.BX:=Ofs(Buf);
   Intr($2F,Regs);
   Lett:='';
   FOR i:=0 TO CDCount-1 DO
     Lett:=Lett+Chr(65+Buf[i+1]);
     GetCDLetters:=Lett
 END;

{ Goi thong bao loi den nguoi dung }
PROCEDURE Send(DiskNum : Byte; VAR Data);
               { DiskNum : So hieu dia }
               { Data    : So vung diem }
VAR
  Regs   : Registers;
  Status : ^Word;

  BEGIN
    Regs.AX:=$1510;
    Regs.CX:=DiskNum;
    Regs.ES:=Seg(Data);
    Regs.BX:=Ofs(Data);
    Intr($2F,Regs);
    Status:=Ptr(Seg(Data),Ofs(Data)+3);
    Error:=Status^ AND $8000<>0;
    Busy:=Status^ AND $0200<>0;
    Ready:=Status^ AND $0100<>0;
    IF Error THEN
      BEGIN
        Writeln('Request error code : ',Lo(Status^));
        Sound(2000);
        Delay(100);
        NoSound;
      END
  END;

{ Khoi tao cac tri so }
PROCEDURE MakeRequest(Command : Byte; VAR Req : Request);
  BEGIN
    Req.Command:=Command;
    Req.ReqLen:=13;
    Req.SuCode:=0;
    Req.Status:=0
  END;

{ Thuc hien cac lenh dieu hanh }
PROCEDURE MakeControl(Command : Byte; VAR Data; NByte : Byte;
                      VAR IO : IOControl);
  BEGIN
    IO.Rh.ReqLen:=13;
    IO.Rh.SuCode:=0;
    IO.Rh.Command:=Command;
    IO.Rh.Status:=0;
    IO.Media:=0;
    IO.Addr:=@Data;
    IO.NByte:=NByte;
    IO.Start:=0
  END;

{ Tinh so Sectors }
PROCEDURE MSF(VAR Ld : Counter);
  BEGIN
    Ld.Sector:=Ld.Min*60;
    Ld.Sector:=(Ld.Sector*75)+(Ld.Sec*75)+Ld.Frame-150
  END;

{ Lay thong tin ve dia nhac }
PROCEDURE GetDiskInfo(DiskNum : Byte; VAR LoT,HiT :Byte);

TYPE
  TBlock=RECORD
           IO   : IOControl;
           Code : Byte;
           Info : DiskInfo
         END;
VAR
  Block   : TBlock;

  BEGIN
    Block.Code:=10;
    MakeControl(ctIn,Block.Code,7,Block.IO);
    Send(DiskNum,Block);
    LoT:=Block.Info.LoTrack;
    HiT:=Block.Info.HiTrack;
    CDInfo:=Block.Info;
    MSF(CDInfo.LeadOut)
  END;

{ Lay thong tin ve tung track }
PROCEDURE GetTrackInfo(DiskNum,TrackNum : Byte; Info :TrackInfo);

TYPE
  TBlock=RECORD
           IO   : IOControl;
           Code : Byte;
           Info : TrackInfo
         END;
VAR
  Block : TBlock;
  Next  : TBlock;

  BEGIN
    Block.Code:=11;
    Block.Info.Track:=TrackNum;
    MakeControl(ctIn,Block.Code,7,Block.IO);
    Send(DiskNum,Block);
    MSF(Block.Info.Start);
    Info:=Block.Info;
    IF TrackNum=CDInfo.HiTrack THEN
      WITH Block.Info,CDInfo.LeadOut DO
      Info.Sectors:=Sector-Start.Sector
    ELSE
      BEGIN
        Next.Code:=11;
        Next.Info.Track:=TrackNum+1;
        MakeControl(ctIn,Next.Code,7,Next.IO);
        Send(DiskNum,Next);
        MSF(Next.Info.Start);
        Info.Sectors:=Next.info.Start.Sector-
        Block.Info.Start.Sector
      END
  END;

{ Bat dau hat nhac }
PROCEDURE PlayCD(DiskNum,TrackNum : Byte);

TYPE
  TBlock=RECORD
           Rh       : Request;
           PlayMode : Byte;
           Start    : LongInt;
           Sectors  : LongInt
         END;
VAR
  Block   : TBlock;
  Info    : TrackInfo;

  BEGIN
    GetTrackInfo(DiskNum,TrackNum,Info);
    Block.Start:=Info.Start.Sector;
    Block.Sectors:=Info.Sectors;
    Block.PlayMode:=0;
    MakeRequest(132,Block.Rh);
    Send(DiskNum,Block)
  END;

{ Dung hat nhac }
PROCEDURE StopCD(DiskNum : Byte);
VAR
  Block : Request;

  BEGIN
    MakeRequest(133,Block);
    Send(DiskNum,Block)
  END;

{ Phuc hoi dia nhac }
PROCEDURE ResumeCD(DiskNum : Byte);
VAR
  Block : Request;

  BEGIN
    MakeRequest(136,Block);
    Send(DiskNum,Block)
  END;

{ Than chuong trinh chinh }
BEGIN
  ClrScr;
  DanhSach:=GetCDLetters;
  IF DanhSach='' THEN
    BEGIN
      Writeln('No Disk in driver E:');
      Write('Please insert a Disk in driver E:');
      Readln;
      Halt
    END;
  DiskNum:=Byte(DanhSach[1])-65; { Lay lai so hieu o dia CDROM }
  Writeln('CD Lettters = ',DanhSach);
  GetDiskInfo(DiskNum,LoT,HiT);
  IF Error THEN EXIT;
  WITH CDinfo.LeadOut DO
    Writeln('Tracks= ',Lot,' .. ',HiT,' Time ',Min,' : ',
    Sec,' Size ',Sector);
    FOR i:=LoT TO HiT DO
      BEGIN
        GetTrackInfo(DiskNum,i,Info);
        WITH Info.Start DO
        Writeln('Track ',i:2,' :Time ',Min:2,' : ',Sec:2,' Start: ',
        Sector:6,' Len: ',Info.Sectors)
      END;
  StopCD(DiskNum); { Stop before Play }
  PlayCD(DiskNum,LoT);
  Write('Playing track ',LoT,' ... ENTER to Stop');
  Readln;
  StopCD(DiskNum);
  Write('Stopped. ENTER to Resume ...');
  Readln;
  ResumeCD(DiskNum);
  Writeln('Complete.');
  Readln
END.
