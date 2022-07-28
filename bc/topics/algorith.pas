{---------------------------------------------------------------------------}
{                      TRUONG DAI HOC KY THUAT CONG NGHE                    }
{                           KHOA CONG NGHE THONG TIN                        }
{                  CHUONG TRINH KIEM TRA CAC THUAT TOAN DON GIAN            }
{                  Written by  : Student NGUYEN NGOC VAN                    }
{                  Mssv        : 00DTH201                                   }
{                  Lop         : 00DTH01                                    }
{                  Update to  : 23/06/2000                                  }
{---------------------------------------------------------------------------}
{  WARNING : Ch¥¤ng tr×nh chØ ch«y ¦¥îc khi cã t¾p tin SVGA256.BGI ¦¥îc     }
{  n«p v§o bé nhí. NÕu b«n kh£ng cã t¾p tin n§y, xin b«n h©y vui lßng li¢n  }
{  hÖ víi tªc gi¨ ¦Ó copy nã. Xin ch¡n th§nh cªm ¤n sù ñng hé quý bªu cña   }
{  cªc b«n. Mäi th¯c m¯t hay ý kiÕn ¦ãng gãp xin li¢n hÖ vÒ ¦Þa chØ sau :   }
{       Address  : BÕn V¡n Ÿån Ph¥êng 1 Qu¾n 4                              }
{       Email    : nguyenvan@yahoo.com                                      }
{       Website  : http://www.unitech.vnn.vn                                }
{       Phone    : 9300061                                                  }
{---------------------------------------------------------------------------}
PROGRAM Simple_Algorithm; { Kiem tra cac thuat toan don gian }
USES Crt,Graph,Dos;

CONST
  BrightShade = White;   { Mau chu bong }
  DarkShade   = DarkGray;{ Mau nen bong }
  Mode16      = TRUE;    { Mode cho he do hoa 16 mau }
  Mode256     = FALSE;   { Mode cho he do hoa 256 mau }

TYPE TableChar = ARRAY[1..8] OF Byte;
     String3   = STRING[3];
     String18  = STRING[18];
VAR
  Table : ARRAY[0..255] OF TableChar ABSOLUTE $F000:$FA6E;
  CenterX             : Word; { Toa do giua }
  XgClot, XdClot, Y,  { Cua so ,tam nhin }
  YbClot, YhClot      : Integer;
  ADeg, ARad, XgFen,  { Cac goc }
  XdFen, YbFen, YhFen,
  Xtl, Ytl, XP1, YP1, { Toa do cat }
  XP2, YP2            : Real;

{----------------------------------------------------------------------}
{             CAC HAM XU LY MAU VAN BAN VA MODE MAN HINH               }
{----------------------------------------------------------------------}

{ Ham lay kich thuoc con tro hien tai }
FUNCTION GetCurSor : Word;
VAR Regs : Registers;
  BEGIN
    Regs.AH:=$03; { So hieu ham }
    Regs.BH:=0;   { Lay kich thuoc con tro hien thoi }
    Intr($10,Regs);
    GetCursor:=Regs.CX; { Lay do lon }
  END;

{ Ham dua ra man hinh mot Symbol voi so lan la n }
FUNCTION Replicate(Symbol :Char; N :Integer) :STRING;
VAR
  Temp : STRING;
  K   : Integer;
  BEGIN
    Temp:='';
    FOR k:=1 TO n DO Temp:=Temp+Symbol;
    Replicate:=Temp
  END;

{ Ham tim so nho nhat trong hai so A va B }
FUNCTION Minium(A,B :Integer) : Integer;
BEGIN
  IF A<B THEN Minium:=A ELSE Minium:=B
END;

{ Ham lay mau chu hien tai }
FUNCTION GetTextColor : Byte;
  BEGIN
    GetTextColor:=TextAttr MOD 16
  END;

{ Ham lay mau nen hien tai }
FUNCTION GetBkColor : Byte;
  BEGIN
    GetBkColor:=TextAttr DIV 16
  END;

{ Ham lay Mode man hinh }
FUNCTION VideoMode : Byte;
VAR Regs : Registers;
  BEGIN
    Regs.AH:=$0F; { So hieu ham }
    Intr($10,Regs);
    VideoMode:=Regs.AL { Mode hien tai cua man hinh }
   END;

{ Ham xac lap kieu cua man hinh }
FUNCTION VidSeg : Word;
  BEGIN
    IF Mem[$0000:$0449]=7 THEN
      VidSeg:=$B000 { Mode cua man hinh Monochrom }
    ELSE
      VidSeg:=$B800 { Mode cua man hinh CGA }
  END;

{ Ham xac dinh dia chi cua cac dong va cot }
FUNCTION OffSet(Col,Row :Byte) :Word;
  BEGIN
    CASE Mem[$0000:$0449] OF
      0,1 : OffSet:=2*(Col-1)+80*(Row-1);
      { Dia chi cua man hinh 40x25 }
    ELSE
      Offset:=2*(Col-1)+160*(Row-1)
      { Dia chi cua man hinh 80x25 }
    END
  END;

{----------------------------------------}
{ Ham cat het khoang trang hai ben chuoi }
{ Vao : (Str) chuoi can cat              }
{ Ra  : Chuoi da cat het khong trong     }
{----------------------------------------}
FUNCTION AllTrim(Str : STRING) : STRING;
BEGIN
  WHILE Str[1]=#32 DO
    Delete(Str,1,1);            { Cat ben trai }
  WHILE Str[Length(Str)]=#32 DO
    Delete(Str,Length(Str),1);  { Cat ben phai }
  AllTrim:=Str
END;


{----------------------------------------------------------------------}
{               CAC MODULE XU LY KICH THUOC CON TRO                    }
{----------------------------------------------------------------------}

{ Dinh kich thuoc con tro }
PROCEDURE SetCurSor(Size : Word);
VAR Regs : Registers;
   BEGIN
     Regs.AH:=1; { So hieu ham }
     Regs.CX:=Size;
     Intr($10,Regs);
   END;

{ Thu tuc dau con tro man hinh }
PROCEDURE CurSorOff;
  BEGIN
    SetCurSor($2020)
  END;

{ Tao con tro co kich thuoc lon }
PROCEDURE BigCurSor;
VAR Vmode : Byte;
   BEGIN
     Vmode:=VideoMode;
     IF Vmode=7 THEN
       SetCurSor($000D)
      ELSE
       SetCurSor($0007);
   END;

{----------------------------------------------------------------------}
{               CAC MODULE XU LY MAU CHU VA XU LY KY TU                }
{----------------------------------------------------------------------}

{ Thu tuc xoa mot phan man hinh voi mau BackGround }
PROCEDURE Clear(x1,y1,x2,y2,BackGround :Byte);
  BEGIN
    Window(x1,y1,x2,y2);
    TextBackGround(BackGround);
    ClrScr;
    Window(1,1,80,25)
  END;

{ Thu tuc xac lap mau vung bien man hinh }
PROCEDURE SetBorderColor(Color :Byte);
VAR Regs : Registers;
  BEGIN
    Regs.AH:=11;
    Regs.BX:=Color;
    Intr($10,Regs)
  END;

{ Thu tuc thiet lap mau chu va mau nen }
PROCEDURE SetTextColor(TxColor,BkColor :Byte);
  BEGIN
    TextBackGround(BkColor);
    TextColor(TxColor)
  END;

{ Thu tuc viet mot chuoi o toa do X,Y }
PROCEDURE WriteXY(x,y :Word; St :STRING);
  BEGIN
    GotoXY(x,y);
    Write(St)
  END;

{ Viet mot chuoi tai toa do x,y co mau
  chu la Tx va mau nen la Bk }
PROCEDURE SayXY(x,y,Tx,Bk : Byte; Message : STRING);
VAR OldTx,OldBk,Len : Byte;
  BEGIN
    OldTx:=GetTextColor; { Lay lai mau chu hien tai}
    OldBk:=GetBkColor;   { Lay lai mau nen hien tai }
    SetTextColor(Tx,Bk);
    Len:=Ord(Message[0])+2;
    GotoXY(x,y);
    Write('':Len);
    WriteXY(x+1,y,Message);
    SetTextColor(OldTx,OldBk)
    { Tra lai cac thong so mau mac nhien ban dau }
  END;

{ Ket thuc chuong trinh }
PROCEDURE EndProgram;
  BEGIN
    SetBorderColor(Black); { Thiet lap mau vien mac dinh   }
    SetCursor($0607);      { Thiet lap con tro mac nhien   }
    TextMode(C80);         { Thiet lap lai che do man hinh }
    ClrScr;                { Xoa man hinh ket qua          }
    Halt(1);               { Dung chuong trinh tro ve HDH  }
  END;

{ Ham lay ky tu o dia cai dat chuong trinh }
FUNCTION GetDisk : STRING3;
VAR Fp                   : TEXT;
    Temp1, Temp2, Driver : STRING[30];
    i                    : Byte;
  BEGIN
     {$I-}
     Assign(Fp,'C:\WINDOWS\register.dat');
     Reset(Fp);
     {$I+}
     IF IOResult <> 0 THEN
        BEGIN
           SetBorderColor(55);
           SetTextColor(Blue,White);
           ClrScr;
           SayXY(30, 10, $0F, $01, 'SYSTEM ERROR!');
           SayXY(20, 12, $0C, $01, 'Cannot reading from file register.dat');
           SayXY(20, 13, $0C, $01, 'Please press ENTER key to exit program');
           Readln;
           EndProgram;
        END;
     Read(Fp, Temp1, Temp2, Driver);
     Close(Fp);
     Temp1:='     ';
     FOR i := 1 TO Length(Driver) DO
        BEGIN
           IF (Driver[i] = #32) OR (Driver[i] IN ['A'..'Z',':','\']) THEN
           Temp1[i] := Driver[i]
        END;
     GetDisk := Alltrim(Temp1);
  END;

{------------ Nhay sang trang ke -----------------}
PROCEDURE NextPage;
VAR I : Integer;
  BEGIN
    GotoXY(1,1);
    FOR I:=1 TO 25 DO
      BEGIN
        Delline;
        Delay(10)
      END
  END;

{ Thu tuc thay doi thuoc tinh cua mot vung man hinh
  voi cac toa do goc tren ben trai la (x1,y1) va goc
  duoi ben phai la (x2,y2) voi mau nen la Bk }
PROCEDURE SetAttrib(x1,y1,x2,y2,Tx,Bk :Byte);
VAR
  Col,Row,Attr :Byte;
  BEGIN
    Attr:=(Bk SHL 4)+Tx; { Thiet lap mau chu va mau nen }
    FOR Col:=x1 TO x2 DO
      FOR Row:=y1 TO y2 DO Mem[VidSeg:OffSet(Col,Row)+1]:=Attr;
      { Bo nho cua man hinh bat dau tu Offset $B800:3E7F }
      { VidSeg=$B800 la dia chi cua man hinh CGA }
      { Dia chi nay thay doi tuy theo man hinh cua ban }
  END;

{ Thiet lap thuoc tinh mau chu va mau nen }
PROCEDURE SetAttribute(BkColor,TxColor : Byte);
  BEGIN
    TextAttr:=(BkColor SHL 4)+TxColor
  END;

{----------------------------------------------------------------------}
{         CAC MODULE XU LY CAC HOP DOI THOAI CUA CHUONG TRINH          }
{----------------------------------------------------------------------}

{ Thu tuc tao ra mot Frame co toa do goc tren ben trai
  la (x1,y1) va goc duoi ben phai la (x2,y2). Tham so
  Lane quyet dinh kieu khung, voi Lane=1 ta co khung
  don, Lane=2 ta co khung kep, Lane=3 ta co khung phia
  tren va duoi kep, Lane=4 ta co khung phia hai ben kep }
PROCEDURE Frame(x1,y1,x2,y2,Lane :Byte);
CONST
  Bound :ARRAY[1..4] OF STRING[6]=
  (#218#196#191#179#217#192,#201#205#187#186#188#200,
  #213#205#184#179#190#212,#214#196#183#186#189#211);
VAR
  Border : STRING[6];
  K      : Integer;

  BEGIN
    Lane:=((Lane-1) MOD 4)+1;
    { De phong truong hop day bang }
    Border:=Bound[Lane];
    WriteXY(x1,y1,Border[1]);
    FOR k:=x1+1 TO x2-1 DO
      Write(Border[2]);
      Write(Border[3]);
      WriteXY(x1,y2,Border[6]);
    FOR k:=x1+1 TO x2-1 DO
      Write(Border[2]);
      Write(Border[5]);
    FOR k:=y1+1 TO y2-1 DO
      BEGIN
        WriteXY(x1,k,Border[4]);
        WriteXY(x2,k,Border[4])
      END
  END;

{ Thu tuc tao ra mot hop co mau nen va co ke khung.
  Tham so Color la mau nen cua hop, Skeleton la mau
  cua khung, Lane la kieu khung (Duoc giai thich o Frame) }
PROCEDURE Box(x1,y1,x2,y2,Color,Skeleton,Lane :Byte);
VAR
  BkColor,TxColor :Byte;
  BEGIN
    TxColor:=GetTextColor; { Lay lai mau chu hien tai }
    BkColor:=GetBkColor;   { Lay lai mau nen hien tai }
    SetTextColor(Skeleton,Color);
    Frame(x1,y1,x2,y2,Lane);
    Window(x1+1,y1+1,x2-1,y2-1);
    ClrScr;
    Window(1,1,80,25);
    { Tra lai mau mac nhien }
    SetTextColor(TxColor,BkColor)
  END;

{ Thu tuc tao ra mot hop co bong rat dep. Voi cac tham so
  Color, Skeleton va Lane duoc giai thich nhu o Box }
PROCEDURE BoxShade(x1,y1,x2,y2,Color,Skeleton,Lane :Byte);
  BEGIN
    Box(x1+2,y1+1,x2+2,y2+1,Black,Black,1);
    Box(x1,y1,x2,y2,Color,Skeleton,Lane)
  END;

{ Thu tuc tao ra mot hop co bong canh rat nghe thuat voi
  cac toa do (x1,y1) va (x2,y2) duoc giai thich nhu tren.
  Tham so Tx la mau chu, Bk la mau nen cua hop. Tham so
  ITx la mau chu cua bong, IBk la mau nen cua bong. Tham
  so Lane quyet dinh kieu cua khung (Xem giai thich o Box)}
PROCEDURE BoxShadow(x1,y1,x2,y2,Tx,Bk,ITx,IBk,Lane :Byte);
  BEGIN
    { Thay doi thuoc tinh vung 1 }
    SetAttrib(x2+1,y1+1,x2+2,y2+1,ITx,IBk);
    { Thay doi thuoc tinh vung 2 }
    SetAttrib(x1+2,y2+1,x2+2,y2+1,ITx,IBk);
    { Tao Box xung quanh vung man hinh bi thay doi }
    Box(x1,y1,x2,y2,Bk,Tx,Lane)
  END;

{----------------------------------------------------------------------}
{             CAC MODULE TRINH BAY DE TRANG TRI MAN HINH               }
{----------------------------------------------------------------------}

{ Thu tuc tao ra dong chu chay tren man hinh }
PROCEDURE RunChar(St : STRING);
VAR
  K   : Integer;
  Tin : STRING;
  BEGIN
    Tin:=Replicate(' ',76)+' '+St+' ';
    FOR k:=1 To Length(Tin) DO
      BEGIN
        TextColor(LightCyan);
        WriteXY(3,24,Copy(Tin,K,Minium(76,Length(Tin)-K+1)));
        Delay(100);
        IF KeyPressed THEN Exit;
      END
  END;

{ Lam day mot vung man hinh bang ky tu ASCII }
PROCEDURE PutBigChar(x,y : Byte; Symbol : Char; CharPaint : Char);
VAR Line,Col    : 1..8;
    CurrentLine : Byte;
    Ch          : TableChar;
    BEGIN
      Ch:=Table[Ord(Symbol)];
      FOR Line:=1 TO 8 DO
        BEGIN
          CurrentLine:=Ch[Line];
          FOR Col:=8 DOWNTO 1 DO
            BEGIN
              IF CurrentLine AND $01<>0 THEN
                WriteXY(x+col-1,y+line-1,CharPaint);
              CurrentLine:=CurrentLine SHR 1
            END
        END
    END;

{ Tao dong chu chay ngang man hinh o muc huong dan
  su dung chuong trinh va muc ban quyen }
PROCEDURE RunChar1;
CONST St = '<< Tr¥êng Ÿ«i Häc Kü Thu¾t C£ng NghÖ TP. Hå ChÝ Minh'+
      '        Khoa C£ng NghÖ Th£ng Tin >>';
  BEGIN
    TextBackGround(DarkGray);
    REPEAT
      Runchar(St)
    UNTIL KeyPressed
  END;

PROCEDURE RunChar2;
CONST St ='<< Ch¥¤ng Tr×nh KiÓm Tra Cªc Thu¾t Toªn Ÿ¤n Gi¨n Version 2.0'+
      ' (c) Copyright By Student NguyÔn Ngäc V¡n All Rights Reserved >>';
  BEGIN
    TextBackGround(DarkGray);
    REPEAT
      RunChar(St)
    UNTIL KeyPressed
  END;

{ Thu tuc viet mot ky tu tai toa do x,y co mau la
  Attr va do dai la Len. Xam nhap vao bo nho man hinh }
PROCEDURE WriteChar(x,y,Attr,Len : Byte; Ch : Char);
VAR
  Vofs : Word;
  I    : Byte;
  BEGIN
    Vofs:=Offset(x,y);
    FOR i:=1 TO Len DO
      BEGIN
        MemW[VidSeg:Vofs]:=(Attr SHL 8) + Ord(Ch);
        Vofs:=Vofs+2
      END
  END;

{ Thuc tuc lam day mot khung co toa do x1,y1,x2,y2
  bang mot ky tu bat ky. Thuong dung de trang tri }
PROCEDURE FillFrame(x1,y1,x2,y2,Attr : Byte; Ch : Char);
VAR Y : Byte;
  BEGIN
    FOR y:=y1 TO y2 DO WriteChar(x1,y,Attr,x2-x1+1,Ch)
  END;

{ Thu tuc viet mot chu noi o toa do X,Y. Tham so Font cho
  biet kieu cua Font, Size la kich thuoc cua Font, tham so
  TextColor la mau chu va BackColor la mau cua bong }
PROCEDURE WriteShade(X, Y : Word; St : STRING; Font, Size, TextColor,
                     BackColor : Integer);
VAR i, h : Byte;
  BEGIN
    SetTextJustify(CenterText,CenterText);
    SetTextStyle(Font,HorizDir,Size);
    IF (Size=0) THEN SetUserCharSize(19,11,1,1);
    H:=TextHeight('M') DIV 10; { Do sau cua bong }
    IF (Size<40) THEN H:=H*2;  { Do cao cua bong }
    SetColor(BackColor);       { De phong Size qua nho }
    FOR i:=H DOWNTO 1 DO OutTextXY(x+i,Y+i,St);
    SetColor(TextColor);
    OutTextXY(x,y,St)
  END;

{ Thu tuc trao doi hai so nguyen X va Y }
PROCEDURE Switch(VAR x,y : Integer);
VAR Temp : Integer;
  BEGIN
    Temp:=x;
    x:=y;
    y:=Temp;
  END;

{ Thu tuc tao ra mot cai nut co canh noi }
PROCEDURE ButtonPlus(x1,y1,x2,y2,Color : Integer);
VAR i, N : Integer;
  BEGIN
    IF x2<x1 THEN Switch(x1,x2);
    IF y2<y1 THEN Switch(y2,y1);
    SetFillStyle(1,Color);
    Bar(x1,y1,x2,y2);
    N:=(x2-x1) DIV 100;
    IF N<2 THEN N:=2;
    IF N>2 THEN N:=3;
    SetColor(DarkShade);
    FOR i:=1 TO N DO
      BEGIN { Ke duong vien toi }
        Line(x1+(i-1),y1+(i-1),x1+(i-1),y2-(i-1));
        Line(x1+(i-1),y2-(i-1),x2-(i-1),y2-(i-1))
      END;
   SetColor(BrightShade);
   FOR i:=1 TO N DO
     BEGIN { Ke duong vien bong }
       Line(x1+(i-1),y1+(i-1),x2-(i-1),y1+(i-1));
       Line(x2-(i-1),y1+(i-1),x2-(i-1),y2-(i-1))
     END;
  END;

{ Thu tuc ta ra cai nut co canh chim }
PROCEDURE ButtonMinus(x1,y1,x2,y2,Color : Integer);
VAR i, N : Integer;
  BEGIN
    IF x2<x1 THEN Switch(x1,x2);
    IF y2<y1 THEN Switch(y2,y1);
    SetFillStyle(1,Color);
    Bar(x1,y1,x2,y2);
    N:=(x2-x1) DIV 100;
    IF N<2 THEN N:=2;
    IF N>2 THEN N:=3;
    SetColor(BrightShade);
    FOR i:=1 TO N DO
      BEGIN { Ke duong vien bong }
        Line(x1+(i-1),y1+(i-1),x1+(i-1),y2-(i-1));
        Line(x1+(i-1),y2-(i-1),x2-(i-1),y2-(i-1))
      END;
    SetColor(DarkShade);
    FOR i:=1 TO N DO
      BEGIN { Ke duong vien toi }
        Line(x1+(i-1),y1+(i-1),x2-(i-1),y1+(i-1));
        Line(x2-(i-1),y1+(i-1),x2-(i-1),y2-(i-1))
      END
  END;

{ Thu tuc tao ra mot nut loi co vien bong }
PROCEDURE ButtonUp(x1,y1,x2,y2,Color1,Color2 : Integer; St : STRING);
VAR I, Xlst : Integer;
  BEGIN
    ButtonPlus(x1,y1,x2,y2,Color1);
    { Xac dinh mau cua nut loi }
    SetColor(White);
    { Mau cua cac ky tu con lai }
    Rectangle(x1-2,y1-2,x2+2,y2+2);
    SetTextStyle(DefaultFont,HorizDir,1);
    SetTextJustify(LeftText,CenterText);
    Xlst:=x1+((x2-x1-TextWidth(st)) DIV 2);
    FOR i:=1 TO Length(st) DO
      IF i=1 THEN
        BEGIN
          SetColor(Red);    { Mau cua ky tu dau tien }
          OutTextXY(Xlst,(y1+y2) DIV 2,St[i])
        END
      ELSE
        BEGIN
          SetColor(Color2); { Xac dinh mau cua bong }
          OutTextXY(Xlst+(i-1)*TextWidth(st[i]),(y1+y2) DIV 2,St[i])
        END
  END;

{ Thu tuc tao ra mot nut lom co vien bong }
PROCEDURE ButtonDown(x1,y1,x2,y2,Color1,Color2 : Integer; St : STRING);
VAR i,Xlst : Integer;
  BEGIN
    ButtonMinus(x1,y1,x2,y2,Color1);
    SetColor(White);
    Rectangle(x1-2,y1-2,x2+2,y2+2);
    SetTextStyle(DefaultFont,HorizDir,1);
    SetTextJustify(LeftText,CenterText);
    Xlst:=x1+((x2-x1-TextWidth(st)) DIV 2);
    FOR i:=1 TO Length(St) DO
      IF i=1 THEN
        BEGIN
          SetColor(Red);
          OutTextXY(Xlst,(y1+y2) DIV 2,St[i])
        END
      ELSE
        BEGIN
          SetColor(Color2);
          OutTextXY(Xlst+(i-1)*TextWidth(St[i]),(y1+y2) DIV 2,St[i])
        END
  END;

{ Thu tuc khoi dong che do do hoa chuan nhat voi tham
  so Path la duong dan toi cac tap tin chua trinh dieu
  khien do hoa (Thuong la cac Files *.BGI va *.CHR) }
PROCEDURE EnterGraph(Mode : Boolean);
VAR
  GraphDriver,GraphMode,
  ErrorCode    : Integer;
  PathToDriver : STRING18;
  BEGIN
    PathToDriver := GetDisk+'TOPICS\GRAPHICS';
    IF Mode THEN { Mode cho he do hoa 16 mau }
      BEGIN
         DetectGraph(GraphDriver,GraphMode);
         { Tu dong kiem tra cac thiet bi do hoa }
         InitGraph(GraphDriver,GraphMode,PathToDriver);
         ErrorCode:=GraphResult;
         IF ErrorCode<>GrOk THEN  { Xay ra loi khoi dong }
            BEGIN
               ClrScr;
               Writeln('Graphics Error : ',GraphErrorMsg(ErrorCode));
               Writeln('Please press ENTER key to exit program now!');
               Readln;
               EndProgram
            END
      END
    ELSE   { Mode cho he do hoa 256 mau }
      BEGIN
         GraphDriver:=InstallUserDriver('SVGA256',NIL);
         GraphMode:=2; { Mode 2 co do phan giai 640x480 }
         InitGraph(GraphDriver,GraphMode,PathToDriver);
         ErrorCode:=GraphResult;
         IF ErrorCode<>GrOK THEN
            BEGIN
               ClrScr;
               Writeln('Graphics Error : ',GraphErrorMsg(ErrorCode));
               Writeln('Please press ENTER key to exit program now!');
               Readln;
               EndProgram
            END
      END
  END; { Ket thuc cac Module thiet ke chuong trinh }

{----------------------------------------------------------------------}
{         CAC MODULE TAO GIAO DIEN MAN HINH CUA CHUONG TRINH           }
{----------------------------------------------------------------------}

{ Thu tuc kiem tra Password de bao ve ban quyen }
PROCEDURE Enter_Password;
VAR
  PassW1, PassW2 : STRING[15];
  Key            : Char;
  i, j           : Byte;
  Flag           : Boolean;
  BEGIN
    PassW1:='NGUYEN NGOC VAN';
    SetTextColor(Yellow,Blue);
    ClrScr;
    REPEAT
      BEGIN
        BigCurSor;
        SetBorderColor(LightGreen);
        FillFrame(1,1,80,25,LightCyan,#178);
        { lap thuoc tinh mau cho ky tu nen }
        SetAttribute($0,$8);
        PutBigChar(5,4,'A',#178);
        PutBigChar(13,4,'L',#178);
        PutBigChar(21,4,'G',#178);
        PutBigChar(30,4,'O',#178);
        PutBigChar(39,4,'R',#178);
        PutBigChar(48,4,'I',#178);
        PutBigChar(55,4,'T',#178);
        PutBigChar(64,4,'H',#178);
        PutBigChar(73,4,'M',#178);
        { Lap thuoc tinh mau cho ky tu chu }
        SetAttribute($0,$D);
        PutBigChar(3,3,'A',#178);
        PutBigChar(11,3,'L',#178);
        PutBigChar(19,3,'G',#178);
        PutBigChar(28,3,'O',#178);
        PutBigChar(37,3,'R',#178);
        PutBigChar(46,3,'I',#178);
        PutBigChar(53,3,'T',#178);
        PutBigChar(62,3,'H',#178);
        PutBigChar(71,3,'M',#178);
        { Tra lai thuoc tinh mac nhien }
        SetAttribute($1,$F);
        CursorOff;
        BoxShadow(20,13,60,17,Yellow,Blue,DarkGray,Black,2);
        SayXY(35,13,White,Red,'Password');
        SayXY(43,15,White,Cyan,'              ');
        SayXY(22,15,Yellow,Blue,'Enter The Password :');
        i:=1;
        SetTextColor(White,Cyan);
        REPEAT
          Key := Readkey;
          IF (i < 16) AND (Key <> #13) AND (Key <> #8) THEN
            BEGIN
              PassW2[i] := Key;
              WriteXY(43+i,15,'*');
              Inc(i)
            END
          ELSE
            IF (Key = #8) AND (i > 1) THEN
              BEGIN
                WriteXY(43+i-1,15,' ');
                Dec(i)
              END
            ELSE
              IF Key <> #13 THEN
                BEGIN
                  Sound(500);
                  Delay(100);
                  Nosound;
                END
        UNTIL Key = #13;
        Flag := TRUE;
        FOR j:=1 TO 15 DO
          IF PassW1[j] <> PassW2[j] THEN Flag := FALSE;
        IF Flag THEN
          BEGIN
            BoxShadow(20,13,60,18,Yellow,LightGray,DarkGray,Black,2);
            SayXY(36,13,White,Red,'Result');
            SayXY(32,15,LightGreen+Blink,LightGray,'CONGRATULATIONS !');
            SayXY(25,16,Blue,LightGray,'Press <ENTER> To Start Program');
            CurSorOff;
            Key:=ReadKey;
            TextMode(LastMode);
            IF Key=#27 THEN Halt
          END
        ELSE
          BEGIN
            BoxShadow(20,13,60,18,Yellow,LightGray,DarkGray,Black,2);
            SayXY(36,13,White,Red,'Result');
            SayXY(31,15,Black+Blink,LightGray,'Incorrect Password');
            SayXY(23,16,Black,LightGray,
            '<ENTER> : Continue / <ESC> : QUIT');
            CurSorOff;
            Key:=ReadKey;
            TextMode(LastMode);
            IF Key=#27 THEN Halt
          END
      END
    UNTIL Flag;
    SetBorderColor(Black)
  END; { Ket thuc thu tuc nhap Password }

{ Thu tuc tao giao dien nam hinh }
PROCEDURE Start_Program;
CONST
  BackColor : Integer=0;
  DrawColor : Integer=1;
VAR
  Color,x1,x2,y1,y2,
  g,r1,r2,i,j,k : Integer;
  Deta,m,n      : Real;
  Regs          : Registers;  { Khoi tao thanh ghi }
  TextMode      : Integer;    { Mode hien tai }

{ Ham lay lai mode hien tao }
FUNCTION GetMode :Integer;
  BEGIN
   Regs.AH:=$F;
   Intr($10,Regs);
   GetMode:=Regs.AL;
  END;

{ Thu tuc thiet lap mode }
PROCEDURE SetMode(Mode :Integer);
  BEGIN
    Regs.AH:=$0;
    Regs.AL:=Mode;
    Intr($10,Regs)
  END;

{ Thu tuc thiet lap mau nen }
PROCEDURE SetBKColor(Color :Integer);
  BEGIN
    IF BackColor<>Color THEN
      BEGIN
        Regs.AH:=$B;
        Regs.BH:=0;
        Regs.BL:=Color;
        Intr($10,Regs)
      END;
  END;

{ Thu tuc xac dinh mau cho chu }
PROCEDURE SetColor(Color :Integer);
  BEGIN
    IF DrawColor<>Color THEN
       DrawColor:=Color;
  END;

{ Thu tuc ve mot diem anh  o toa do X,Y  mau la Color }
PROCEDURE PutPixel(x,y :Word; Color :Integer);
  BEGIN
     Regs.AH:=$C;
     Regs.AL:=Color;
     Regs.CX:=X;
     Regs.DX:=Y;
     Regs.BH:=0;
     Intr($10,Regs)
   END;

{ Ham lay lai toa do diem anh hien tai }
FUNCTION GetPixel(x,y :Integer) :Integer;
  BEGIN
    Regs.AH:=$D;
    Regs.DX:=X;
    Regs.CX:=Y;
    Regs.BH:=0;
    Intr($10,Regs);
    GetPixel:=Regs.AL
  END;

{ Thu tuc xuat mot chuoi ra man hinh o che do Graphics }
PROCEDURE OutTextXY(x,y : Byte; Ch : STRING);
VAR
  i, k : Byte;
  BEGIN
    FOR i:=1 TO length(ch) DO
      BEGIN
        Regs.AH:=2;
        Regs.DH:=Y;
        Regs.DL:=X;
        Regs.BH:=0;
        Intr($10,Regs);
        Regs.AH:=9;
        Regs.AL:=Ord(Ch[i]);
        Regs.BL:=DrawColor;
        Regs.CX:=1;
        Intr($10,Regs);
        x:=x+1
      END
  END;

{ Thu tuc ve mot doan thang trong che do Graphics
  voi (x1,y1) la toa do dau cua doan thang va (x2,
  y2) la toa do cuoi cua doan thang ( Ve bang thu
  tuc PutPixel da duoc dinh nghia o tren }
PROCEDURE Line(x1,y1,x2,y2 :Integer);
VAR
  Dx,Dy,x_Inc,y_Inc,
  x,y,p,Const1,Const2,
  i,Increment : Integer;
   BEGIN
     Dx:=x2-x1;
     Dy:=y2-y1;
     IF Dx>0 THEN x_Inc:=1
     ELSE
       IF Dx<0 THEN x_Inc:=-1
       ELSE x_Inc:=0;
         IF Dy>0 THEN y_Inc:=1
         ELSE
           IF Dy<0 THEN y_Inc:=-1
           ELSE y_Inc:=0;
     Dx:=Abs(Dx);
     Dy:=Abs(Dy);
     IF Dx>=Dy THEN
       BEGIN
         p:=2*Dy-Dx;
         Const1:=2*Dy;
         Const2:=2*(Dy-Dx);
         Increment:=Dx
       END
     ELSE
       BEGIN
         p:=2*Dx-Dy;
         Const1:=2*Dx;
         Const2:=2*(Dx-Dy);
         Increment:=Dy
       END;
      i:=0;
      x:=x1; y:=y1 ;
      PutPixel(Round(x),Round(y),DrawColor);
      WHILE i<=Increment DO
        BEGIN
          IF Dx>=Dy THEN x:=x+x_Inc
          ELSE y:=y+y_Inc;
          IF p<0 THEN p:=p+Const1
          ELSE
            BEGIN
              IF Dx>=Dy THEN y:=y+y_Inc
              ELSE x:=x+x_Inc;
             p:=p+Const2
            END;
         PutPixel(Round(x),Round(y),DrawColor);
         Inc(i)
        END { While }
   END;

{ Thu tuc ve duong tron trong che do Graphics
  Chu y: Circle ve bang thuat toan Bressenham }
PROCEDURE Circle(Xc,Yc, R : Integer);
VAR x,y,p : Integer;

  PROCEDURE Doi_Xung; { Thu tuc Xac dinh tinh doi xung }
    BEGIN
      PutPixel(Round(Xc+x),Round(Yc+y),DrawColor);
      PutPixel(Round(Xc+x),Round(Yc-y),DrawColor);
      PutPixel(Round(Xc-x),Round(Yc+y),DrawColor);
      PutPixel(Round(Xc-x),Round(Yc-y),DrawColor);
      PutPixel(Round(Xc+y),Round(Yc+x),DrawColor);
      PutPixel(Round(Xc-y),Round(Yc+x),DrawColor);
      PutPixel(Round(Xc+y),Round(Yc-x),DrawColor);
      PutPixel(Round(Xc-y),Round(Yc-x),DrawColor)
    END;

  BEGIN   { Bat dau thu tuc Circle }
    x:=0; y:=r;
    p:=3-2*r;
    WHILE x<y DO
      BEGIN
        Doi_Xung;
        IF p<0 THEN p:=p+4*x+6
        ELSE
          BEGIN
            p:=p+4*(x-y)+10;
            y:=y-1
          END;
         x:=x+1
      END; { While }
     IF x=y THEN Doi_Xung;
  END; { Bres_Circle }

{ Thu tuc ve mot Ellipse trong che do Graphics
  Chu y : Ellipse duoc ve bang thuat toan Bressenham }
PROCEDURE Ellipse(Xc,Yc,Ar,Br :Integer);
VAR
  x,y     : Integer;
  z1,z2,p : Real;
  BEGIN
    x:=0; y:=br;
    z1:=(Ar*Ar)/(Br*Br);
    z2:=(Br*Br)/(Ar*Ar);
    p:=2*z2-2*Br+1;
    WHILE (z2*(x/y)<=1) DO
      BEGIN
        PutPixel(Round(Xc+x),Round(Yc+y),DrawColor);
        PutPixel(Round(Xc+x),Round(Yc-y),DrawColor);
        PutPixel(Round(Xc-x),Round(Yc+y),DrawColor);
        PutPixel(Round(Xc-x),Round(Yc-y),DrawColor);
        IF p<0 THEN p:=p+2*z2*(2*x+3)
        ELSE
          BEGIN
            p:=p+2*z2*(2*x+3)+4*(1-y);
            y:=y-1
          END;
        x:=x+1;
      END; { While }
    x:=ar; y:=0;
    p:=2*z1-2*ar+1;
    WHILE (z1*(y/x)<=1) DO
      BEGIN
        PutPixel(Round(Xc+x),Round(Yc+y),DrawColor);
        PutPixel(Round(Xc+x),Round(Yc-y),DrawColor);
        PutPixel(Round(Xc-x),Round(Yc+y),DrawColor);
        PutPixel(Round(Xc-x),Round(Yc-y),DrawColor);
        IF p<0 THEN p:=p+2*z1*(2*y+3)
        ELSE
          BEGIN
            p:=p+2*z1*(2*y+3)+4*(1-x);
            x:=x-1
          END;
        y:=y+1
      END; { While }
     END; { Bres_Ellipse }

BEGIN    { Bat dau thu tuc tao giao dien man hinh }
  EnterGraph(Mode16);
  k:=1;
  REPEAT { Ve bau troi day sao va chuoi roi tu do }
    IF k=30 THEN
      BEGIN
        k:=1;
        j:=Random(54)
      END;
    SetColor(Random(MaxColors)+1);
    OutTextXY(j,k,'WELCOME TO MY PROGRAMME !');
    Delay(50);
    OutTextXY(j,k,'                         ');
    IF Random(2)=0 THEN k:=k+1;
    PutPixel(Random(GetMaxX),Random(GetMaxY),Random(MaxColors)+1);
    x1:=Random(GetMaxX);
    y1:=Random(GetMaxY);
    SetColor(Random(MaxColors)+1);
    Line(x1-4,y1,x1+4,y1);
    Line(x1,y1-2,x1,y1+2);
    FOR i:=0 TO 100 DO
      BEGIN
        x1:=Random(GetMaxX);
        y1:=Random(GetMaxY);
        Setcolor(BackColor);
        Line(x1-4,y1,x1+4,y1);
        Line(x1,y1-2,x1,y1+2)
      END;
  UNTIL KeyPressed;
  Readln;
  K:=210;
  ClearDevice;
  SetBKColor(DarkGray);
  REPEAT { Ve hinh da quang }
    SetColor(Random(MaxColors)+1);
    FOR i:=1 TO 16 DO Circle(GetMaxX DIV 2,GetMaxY DIV 2,i);
    r1:=20;
    WHILE (r1<=310) DO
      BEGIN
        r2:=r1+10;
        g:=0;
        WHILE g<=90 DO
          BEGIN
            Deta:=3.14*g/180;
            m:=Sin(Deta);
            n:=Cos(Deta);
            y1:=Round(r1*m);
            y2:=Round(r2*m);
            x1:=Round(r1*n);
            x2:=Round(r2*n);
            Line(320+x1,240+y1,320+x2,240+y2);
            Line(320-x1,240+y1,320-x2,240+y2);
            Line(320+x1,240-y1,320+x2,240-y2);
            Line(320-x1,240-y1,320-x2,240-y2);
            g:=g+9
          END; { While }
        r1:=r1+20
      END { While }
  UNTIL KeyPressed;
  Readln;
  REPEAT  { Ve cac duong tron di chuyen ngau nhien }
    k:=Random(100)+20;
    SetMode(GetMode);
    IF Random(2)=0 THEN
      BEGIN
        IF Random(2)=0 THEN
          BEGIN
            i:=k;
            j:=Random(GetMaxY-2*k)+k;
            IF Random(2)=0 THEN
              WHILE i<GetMaxX-k DO
                 BEGIN
                   Circle(i,j,k);
                   SetColor(Random(MaxColors)+1);
                   i:=i+10
                 END
            ELSE
              BEGIN
                i:=GetMaxX-k;
                WHILE i>k DO
                  BEGIN
                    i:=i-10;
                    Circle(i,j,k);
                    SetColor(Random(MaxColors)+1)
                  END
              END
          END
        ELSE
          BEGIN
            i:=k;
            j:=Random(GetMaxX-2*k)+k;
            IF Random(2)=0 THEN
              WHILE i<GetMaxY - k DO
                BEGIN
                  Circle(j,i,k);
                  SetColor(Random(MaxColors)+1);
                  i:=i+10
                END
            ELSE
              BEGIN
                i:=GetMaxY-k;
                WHILE i>k DO
                  BEGIN
                    i:=i-10;
                    Circle(j,i,k);
                    SetColor(Random(MaxColors)+1)
                  END
              END
          END
      END
    ELSE
      BEGIN
        IF Random(2)=0 THEN
          BEGIN
            i:=k;
            j:=Random(GetMaxY-2*k)+k;
            IF Random(2)=0 THEN
              WHILE i<GetMaxX-k DO
                BEGIN
                  Ellipse(i,j,k,k-10);
                  SetColor(Random(MaxColors)+1);
                  i:=i+10
                END
            ELSE
              BEGIN
                i:=GetMaxX-k;
                WHILE i>k DO
                  BEGIN
                    i:=i-10;
                    Ellipse(i,j,k,k-10);
                    SetColor(Random(MaxColors)+1)
                  END
              END
          END
        ELSE
          BEGIN
            i:=k;
            j:=Random(GetMaxX-2*k)+k;
            IF Random(2)=0 THEN
              WHILE i<GetMaxY-k DO
                BEGIN
                  Ellipse(j,i,k,k-10);
                  SetColor(Random(MaxColors)+1);
                  i:=i+10
                END
            ELSE
              BEGIN
                i:=GetMaxY-k;
                WHILE i>k DO
                  BEGIN
                    i:=i-10;
                    Ellipse(j,i,k,k-10);
                    SetColor(Random(MaxColors)+1)
                  END
              END
          END
      END
  UNTIL KeyPressed;
  CloseGraph;
  Readln
END; { Ket thuc thu tuc StartProgram }

{----------------------------------------------------------------------}
{             CAC MODULE THIET KE GIOI THIEU CHUONG TRINH              }
{----------------------------------------------------------------------}

{ Thiet lap nen noi de trinh bay cac thong bao }
PROCEDURE Khung_Nen_1;
  BEGIN
    { Khung hinh chu nhat ben ngoai }
    SetFillStyle(SolidFill,LightGray);
    Bar(0,10,GetMaxX-1,GetMaxY-1);
    Setcolor(White);
    Rectangle(0,10,GetMaxX,GetMaxY);
    { Net ve ben trai }
    SetColor(Black);
    Moveto(1,11);
    Lineto(1,478);
    Lineto(7,472);
    Lineto(7,17);
    Lineto(1,11);
    { Net ve ben phai }
    SetFillStyle(SolidFill,DarkGray);
    SetColor(DarkGray);
    Moveto(638,11);
    Lineto(638,478);
    Lineto(632,472);
    Lineto(632,17);
    Lineto(638,11);
    FloodFill(635,20,DarkGray);
    { Net ve ben tren }
    Moveto(1,11);
    Lineto(638,11);
    Lineto(632,17);
    Lineto(7,17);
    Lineto(1,11);
    { Net ve ben duoi }
    SetFillStyle(SolidFill,DarkGray);
    SetColor(DarkGray);
    Moveto(1,478);
    Lineto(638,478);
    Lineto(632,472);
    Lineto(7,472);
    Lineto(1,478);
    FloodFill(10,475,DarkGray);
    { Ve khung hinh nen ben trong }
    SetFillStyle(SolidFill,LightBlue);
    Bar(8,18,631,471);
  END;

{ Hien ra dong mo dau chuong trinh }
PROCEDURE Introduce_1;
  BEGIN
    EnterGraph(Mode256);
    Khung_Nen_1;
    CenterX:=GetMaxX DIV 2;
    Y:=GetMaxY DIV 8;
    WriteShade(CenterX,y+20,'UNIVERSITY OF',DefaultFont,5,LightRed,Yellow);
    WriteShade(CenterX,y+120,'TECHNOLOGY',DefaultFont,5,LightRed,Yellow);
    WriteShade(CenterX,y+220,'LUAN AN CUOI KY',DefaultFont,5,LightGreen,
               LightMagenta);
    WriteShade(CenterX,y+300,'KHOA',DefaultFont,4,LightRed,LightCyan);
    WriteShade(CenterX,y+370,'CONG NGHE THONG TIN',DefaultFont,4,LightRed,
               LightCyan);
    Readln;
    CloseGraph
  END;

{ Tao khung nen cho Introduce 2 }
PROCEDURE Khung_Nen_2;
  BEGIN
    { Khung hinh chu nhat ben ngoai }
    SetFillStyle(SolidFill,LightGray);
    Bar(0,10,GetMaxX-1,GetMaxY-1);
    Setcolor(White);
    Rectangle(0,10,GetMaxX,GetMaxY);
    { Net ve ben trai }
    SetColor(Black);
    Moveto(1,11);
    Lineto(1,478);
    Lineto(7,472);
    Lineto(7,17);
    Lineto(1,11);
    { Net ve ben phai }
    SetFillStyle(SolidFill,DarkGray);
    SetColor(DarkGray);
    Moveto(638,11);
    Lineto(638,478);
    Lineto(632,472);
    Lineto(632,17);
    Lineto(638,11);
    FloodFill(635,20,DarkGray);
    { Net ve ben tren }
    Moveto(1,11);
    Lineto(638,11);
    Lineto(632,17);
    Lineto(7,17);
    Lineto(1,11);
    { Net ve ben duoi }
    SetFillStyle(SolidFill,DarkGray);
    SetColor(DarkGray);
    Moveto(1,478);
    Lineto(638,478);
    Lineto(632,472);
    Lineto(7,472);
    Lineto(1,478);
    FloodFill(10,475,DarkGray);
    { Ve khung hinh nen ben trong }
    SetFillStyle(SolidFill,Magenta);
    Bar(8,18,631,471);
  END;

{ Man hinh gioi thieu 2 }
PROCEDURE Introduce_2;
  BEGIN
    EnterGraph(Mode256);
    Khung_Nen_2;
    CenterX:=GetMaxX DIV 2;
    y:=GetMaxY DIV 8;
    WriteShade(CenterX,y,'CHUONG TRINH KIEM TRA',DefaultFont,3,LightBlue,
               White);
    WriteShade(CenterX,y+50,'CAC THUAT TOAN',DefaultFont,3,LightBlue,White);
    WriteShade(CenterX,y+100,'DON GIAN',DefaultFont,3,LightBlue,White);
    WriteShade(CenterX,y+160,'VERSION 2.0',DefaultFont,4,LightMagenta,
               Yellow);
    WriteShade(CenterX,y+310,'NGUYEN NGOC VAN',DefaultFont,5,LightRed,
               LightCyan);
    WriteShade(CenterX,y+220,'(c) Copyright 2000 By',SmallFont,10,White,
               Cyan);
    WriteShade(CenterX,y+370,'Lop 00DTH01',TriplexFont,5,White,
               LightMagenta);
    Readln;
    CloseGraph
  END;

{----------------------------------------------------------------------}
{         CAC MODULE LIEM TRA CAC THUAT TOAN CUA CHUONG TRINH          }
{----------------------------------------------------------------------}

PROCEDURE Module_1; { Giai phuong trinh bac nhat }
VAR
  A,B    : Integer;
  Traloi : Char;
  St1    : STRING;

  PROCEDURE Equation_First(a,b : Integer);
     PROCEDURE A_Bang_0;
       BEGIN
         IF a=0 THEN
           BEGIN
             IF b=0 THEN
               BEGIN
                 Clear(27,9,55,16,Blue);
                 BoxShade(26,9,51,14,Magenta,White,2);
                 SayXY(35,9,White,Green,'Result');
                 SetTextColor(Yellow,Magenta);
                 WriteXY(28,11,'Ph¥¤ng tr×nh cã nghiÖm');
                 WriteXY(30,12,'víi mäi X thuéc R')
               END
             ELSE
               BEGIN
                 Clear(27,9,55,16,Blue);
                 BoxShade(26,9,51,14,Magenta,White,2);
                 SayXY(35,9,White,Green,'Result');
                 SetTextColor(LightCyan+Blink,Magenta);
                 WriteXY(35,11,'WARNING!');
                 SetTextColor(Yellow,Magenta);
                 WriteXY(28,12,'Ph¥¤ng tr×nh v£ nghiÖm')
               END
           END
       END;

     PROCEDURE A_Khac_0;
     VAR N : Real;
       BEGIN
         N:=-b/a;
         Clear(27,9,55,16,Blue);
         BoxShade(26,9,51,14,Magenta,White,2);
         SayXY(35,9,White,Green,'Result');
         SetTextColor(Yellow,Magenta);
         WriteXY(28,11,'Ph¥¤ng tr×nh cã nghiÖm');
         WriteXY(30,12,'* X=');
         GotoXY(35,12);
         Write(N:0:2)
       END;

    BEGIN  { Bat dau PROCEDURE Equation_First }
      IF a=0 THEN A_Bang_0
      ELSE A_Khac_0
    END;   { End of Equation_First }

BEGIN   { Bat dau Module_1 }
  REPEAT
    SetTextColor(Yellow,Blue);
    SetBorderColor(Yellow);
    ClrScr;
    BigCurSor;
    Box(18,2,62,4,Brown,White,2);
    SayXY(20,3,LightCyan,Brown,'Ch¥¤ng Tr×nh Gi¨i Ph¥¤ng Tr×nh B¾c Nh½t');
    BoxShade(27,9,53,14,Cyan,Yellow,2);
    SayXY(34,9,White,Red,'Input Data');
    SetTextColor(Black,Cyan);
    WriteXY(29,11,'- Nh¾p hÖ sè A = ');
    Readln(a);
    WriteXY(29,12,'- Nh¾p hÖ sè B = ');
    Readln(b);
    Equation_First(a,b);
    SetTextColor(Yellow,Blue);
    WriteXY(26,17,'Do you want to continue (Y/N)? ');
    Readln(Traloi)
  UNTIL Upcase(Traloi)='N';
  SetBorderColor(Black)
END; { End of Module_1 }

{---------------------------------------------------------------}
{   Phuc vu cho Module_2. Day la phuong phap tham khao truoc    }
                PROCEDURE Main_Menu; FORWARD;
{  sau do moi dinh nghia de tro ve Menu chinh cua chuong trinh  }
{---------------------------------------------------------------}

PROCEDURE Module_2;  { Giai phuong trinh bac hai }
VAR
  A,B,C  : Integer;
  Traloi : Char;

  PROCEDURE A_Khong;
    BEGIN
      Clear(27,9,59,16,Blue);
      BoxShade(23,9,58,16,Magenta,White,2);
      SayXY(37,9,White,Green,'Result');
      SayXY(36,11,LightCyan+Blink,Magenta,'WARNING!');
      SetTextColor(Yellow,Magenta);
      WriteXY(30,12,'Testing Please wait ..!');
      CurSorOff;
      Sound(2000);
      Delay(2000);
      NoSound;
      WriteXY(25,13,'Kh£ng ph¨i Ph¥¤ng tr×nh b¾c hai!');
      WriteXY(25,14,'Press <ENTER> To Back Main Menu');
      Readln;
      Main_Menu
    END;

  PROCEDURE Equation_Second(A,B,C : Integer);
  VAR Delta : Real;

    PROCEDURE Delta_Khong;
    VAR Ngiem_kep :Real;
      BEGIN
        Ngiem_Kep:=-b/(2*a);
        Clear(27,9,59,16,Blue);
        BoxShade(25,9,57,14,Magenta,White,2);
        SayXY(37,9,White,Green,'Result');
        SetTextColor(Yellow,Magenta);
        WriteXY(29,11,'Ph¥¤ng tr×nh cã nghiÖm kÐp');
        WriteXY(31,12,'* X=');
        GotoXY(35,12);
        Write(Ngiem_Kep:0:2)
      END;

    PROCEDURE Delta_Am;
      BEGIN
        Clear(27,9,59,16,Blue);
        BoxShade(26,9,54,14,Magenta,White,2);
        SayXY(37,9,White,Green,'Result');
        SayXY(36,11,LightCyan+Blink,Magenta,'WARNING!');
        SayXY(29,12,Yellow,Magenta,'Ph¥¤ng tr×nh v£ nghiÖm')
      END;

    PROCEDURE Delta_Duong;
    VAR X1,X2 :Real;
      BEGIN
        X1:=(-b+Sqrt(Delta))/(2*a);
        X2:=(-b-Sqrt(Delta))/(2*a);
        Clear(27,9,59,16,Blue);
        BoxShade(25,9,57,15,Magenta,White,2);
        SayXY(37,9,White,Green,'Result');
        SetTextColor(Yellow,Magenta);
        WriteXY(29,11,'Ph¥¤ng tr×nh cã hai nghiÖm');
        WriteXY(31,12,'* X1=');
        WriteXY(31,13,'* X2=');
        GotoXY(36,12);
        Write(X1:0:2);
        GotoXY(36,13);
        Write(X2:0:2)
      END;

    BEGIN { Bat dau PROCEDURE Equation_Second }
      Delta:=SQR(b)-4*(a*c);
      IF A=0 THEN A_Khong;
      IF Delta=0 THEN Delta_Khong;
      IF Delta>0 THEN Delta_Duong;
      IF Delta<0 THEN Delta_Am
    END;  { End of Equation_Second }

BEGIN  { Bat dau Module_2 }
  REPEAT
    SetTextColor(Yellow,Blue);
    SetBorderColor(Yellow);
    ClrScr;
    BigCurSor;
    Box(19,2,62,4,Brown,White,2);
    SayXY(21,3,LightCyan,Brown,'Ch¥¤ng Tr×nh Gi¨i Ph¥¤ng Tr×nh B¾c Hai');
    BoxShade(27,9,53,15,Cyan,Yellow,2);
    SayXY(34,9,White,Red,'Input Data');
    SetTextColor(Black,Cyan);
    WriteXY(29,11,'- Nh¾p hÖ sè A = ');
    Readln(a);
    WriteXY(29,12,'- Nh¾p hÖ sè B = ');
    Readln(b);
    WriteXY(29,13,'- Nh¾p hÖ sè C = ');
    Readln(c);
    Equation_Second(a,b,c);
    SetTextColor(Yellow,Blue);
    WriteXY(26,18,'Do you want to continue (Y/N)? ');
    Readln(Traloi)
  UNTIL Upcase(Traloi)='N';
  SetBorderColor(Black)
END; { End of Module_2 }

PROCEDURE Module_3 ; { Giai he phuong trinh tuyen tinh }
VAR
  a1,a2,b1,
  b2,c1,c2 : Integer;
  Dx,Dy,DD : Real;
  Traloi   : Char;

BEGIN { Bat dau Module_3 }
  REPEAT
    SetTextColor(Yellow,Blue);
    SetBorderColor(Yellow);
    ClrScr;
    BigCurSor;
    Box(16,2,65,4,Brown,White,2);
    SayXY(18,3,LightCyan,Brown,
          'Ch¥¤ng Tr×nh Gi¨i HÖ Ph¥¤ng Tr×nh TuyÕn TÝnh');
    BoxShade(27,9,54,18,Cyan,Yellow,2);
    SayXY(35,9,White,Red,'Input Data');
    SetTextColor(Black,Cyan);
    WriteXY(29,11,'- Nh¾p hÖ sè A1 = ');
    Readln(a1);
    WriteXY(29,12,'- Nh¾p hÖ sè B1 = ');
    Readln(b1);
    WriteXY(29,13,'- Nh¾p hÖ sè C1 = ');
    Readln(c1);
    WriteXY(29,14,'- Nh¾p hÖ sè A2 = ');
    Readln(a2);
    WriteXY(29,15,'- Nh¾p hÖ sè B2 = ');
    Readln(b2);
    WriteXY(29,16,'- Nh¾p hÖ sè C2 = ');
    Readln(c2);
    SetTextColor(Yellow,Blue);
    DD:=a1*b2-a2*b1;
    Dx:=b1*c2-b2*c1;
    Dy:=a2*c1-a1*c2;
    IF DD = 0 THEN
      BEGIN
        IF (Dx = 0) AND (Dy = 0) THEN
          BEGIN
            Clear(27,9,59,20,Blue);
            BoxShade(25,9,56,15,Magenta,White,2);
            SetTextColor(White,Red);
            SayXY(37,9,White,Green,'Result');
            SayXY(36,11,LightCyan+Blink,Magenta,'WARNING!');
            SayXY(30,12,Yellow,Magenta,'*** HÖ V£ ŸÞnh ***');
            SetTextColor(Yellow,Blue);
            WriteXY(25,18,'Do you want to continue (Y/N)? ');
            Readln(Traloi)
          END
        ELSE
          BEGIN
            Clear(27,9,59,20,Blue);
            BoxShade(25,9,56,15,Magenta,White,2);
            SayXY(37,9,White,Green,'Result');
            SayXY(37,11,LightCyan+Blink,Magenta,'WARNING!');
            SayXY(31,12,Yellow,Magenta,'*** HÖ V£ NghiÖm ***');
            SetTextColor(Yellow,Blue);
            WriteXY(25,18,'Do you want to continue (Y/N)? ');
            Readln(Traloi)
          END
      END
    ELSE
      BEGIN
        Clear(27,9,59,20,Blue);
        BoxShade(25,9,57,15,Magenta,White,2);
        SayXY(37,9,White,Green,'Result');
        SetTextColor(Yellow,Magenta);
        WriteXY(28,11,'NghiÖm cña hÖ ph¥¤ng tr×nh');
        WriteXY(31,12,'* X=');
        WriteXY(31,13,'* Y=');
        GotoXY(36,12);
        Write((Dx/DD):0:2);
        GotoXY(36,13);
        Write((Dy/DD):0:2);
        SetTextColor(Yellow,Blue);
        WriteXY(25,18,'Do you want to continue (Y/N)? ');
        Readln(Traloi)
      END
  UNTIL UpCase(Traloi)='N';
  SetBorderColor(Black)
END; { End of Module_3 }

PROCEDURE Module_4; { Tinh giai thua cua mot so nguyen duong }
VAR
  N,i       : Integer;
  Giai_thua : LongInt;
  Traloi    : Char;

BEGIN
  REPEAT
    SetTextColor(Yellow,Blue);
    SetBorderColor(Yellow);
    ClrScr;
    BigCurSor;
    Box(19,2,62,4,Brown,White,2);
    SayXY(21,3,LightCyan,Brown,'Ch¥¤ng Tr×nh TÝnh Giai Thõa Cña Mét Sè');
    BoxShade(27,9,53,13,Cyan,Yellow,2);
    SayXY(34,9,White,Red,'Input Data');
    SetTextColor(Black,Cyan);
    WriteXY(29,11,'Cho giai thõa N = ');
    Readln(n);
    IF n < 0 THEN
      BEGIN
        Clear(27,9,53,13,Blue);
        BoxShade(25,9,57,15,Magenta,White,2);
        SayXY(37,9,White,Green,'Result');
        SayXY(37,11,LightCyan+Blink,Magenta,'WARNING!');
        SetTextColor(Yellow,Magenta);
        WriteXY(29,12,'Kh£ng tÝnh ¦¥îc giai thõa');
        WriteXY(32,13,'v× N l§ mét sè ¡m')
      END  { Of N<0 }
    ELSE
      BEGIN
        Giai_thua:=1;
        FOR i:=1 TO n DO Giai_thua:=Giai_thua*i;
        Clear(27,9,53,13,Blue);
        BoxShade(25,9,57,13,Magenta,White,2);
        SayXY(37,9,White,Green,'Result');
        SetTextColor(Yellow,Magenta);
        WriteXY(27,11,'Giai thõa cña');
        GotoXY(41,11);
        Write(n,' = ',Giai_thua)
      END;  { Of FOR }
    SetTextColor(Yellow,Blue);
    WriteXY(25,18,'Do you want to continue (Y/N)? ');
    Readln(Traloi)
  UNTIL UpCase(Traloi)='N';
  SetBorderColor(Black)
END; { End of Module_4 }

PROCEDURE Module_5; { Tinh X mu Y voi X la so nguyen duong }
VAR
  X,Y,Z  : Real;
  Traloi : Char;

BEGIN   { Bat dau Module_5 }
  REPEAT
    SetTextColor(Yellow,Blue);
    SetBorderColor(Yellow);
    ClrScr;
    BigCurSor;
    Box(25,2,54,4,Brown,White,2);
    SayXY(27,3,LightCyan,Brown,'Ch¥¤ng Tr×nh TÝnh X Mò Y');
    BoxShade(27,9,53,14,Cyan,Yellow,2);
    SayXY(34,9,White,Red,'Input Data');
    SetTextColor(Black,Cyan);
    WriteXY(29,11,'- Nh¾p hÖ sè X = ');
    Readln(X);
    IF x<=0 THEN
      BEGIN
        Clear(27,9,54,15,Blue);
        BoxShade(24,9,56,15,Magenta,White,2);
        SayXY(36,9,White,Green,'Result');
        SayXY(36,11,LightCyan+Blink,Magenta,'WARNING!');
        SetTextColor(Yellow,Magenta);
        WriteXY(28,12,'Kh£ng tÝnh ¦¥îc X mò Y');
        WriteXY(35,13,'v× X <=0 ')
      END  { Of x<=0 }
    ELSE
      BEGIN
        WriteXY(29,12,'- Nh¾p hÖ sè Y = ');
        Readln(y);
        Z:=EXP(y*LN(x));
        Clear(27,9,55,16,Blue);
        BoxShade(24,9,56,13,Magenta,White,2);
        SayXY(36,9,White,Green,'Result');
        SetTextColor(Yellow,Magenta);
        GotoXY(28,11);
        Write(X:0:2,' Mò ',y:0:2,' = ',Z:0:4)
      END;  {ELSE Of X<=0}
    SetTextColor(Yellow,Blue);
    WriteXY(24,18,'Do you want to continue (Y/N)? ');
    Readln(Traloi)
  UNTIL UpCase(Traloi)='N';
  SetBorderColor(Black)
END; { End of Module_5 }

{----------------------------------------------------------------------}
{           CAC MODULE THIET KE CAC CHUONG TRINH DO HOA DEMO           }
{----------------------------------------------------------------------}

{ Thu tuc tao ra cac ky tu co mau thay doi ngau nhien }
PROCEDURE Graph_Demo0;
VAR
  Key   : Char;
  Part  : LongInt;

{ Khoi tao chuong trinh }
PROCEDURE StartProgram;
  BEGIN
    SetTextColor(Yellow,Blue);
    ClrScr;
    WriteXY(28,18,'Press Any Key To Continue');
    Frame(2,1,79,25,1);
    Clear(29,4,51,14,0);
    BoxShade(29,4,51,14,Brown,White,2)
  END;

{ Thu tuc thiet lap mot Box }
PROCEDURE CBox(x1,y1,x2,y2,Color :Byte);
VAR i : Integer;
  BEGIN
    TextColor(Color);
    FOR i:=x1 TO x2 DO
      BEGIN
        WriteXY(i,y1,#178);
        WriteXY(i,y2,#178)
      END;
    FOR i:=y1+1 TO y2-1 DO
      BEGIN
        WriteXY(x1,i,#178);
        WriteXY(x2,i,#178)
      END;
  END;

{ Thu tuc tao ra cac hop mau co mau thay doi ngau nhien }
PROCEDURE Demo4;
VAR i,j : Integer;
  BEGIN
    Part:=Random(15);
    REPEAT
      FOR i:=1 TO  5 DO
        CBox(29+i,4+i,51-i,14-i,((i MOD 16)+Part) MOD 16);
      Delay(100);
      Inc(Part);
    UNTIL KeyPressed;
    Key:=ReadKey
  END;

BEGIN { Bat dau thu tuc Graph_Demo0 }
  StartProgram;
  Demo4
END;  { Ket thuc PROCEDURE Graph_Demo0}

{ Thu tuc tao chu to trong che do Text }
PROCEDURE Graph_Demo1;
CONST Left  = TRUE;
      Right = FALSE;

VAR
  St    : STRING;
  LowSt : STRING;
  Row   : Byte;

{ Thut uc lay cac ky tu dac biet de tao ra chu to }
PROCEDURE PaintBigChar(Entry :TableChar);
VAR PatternLine,Spot : 1..8;
    CurrentLine      : Byte;
  BEGIN
    FOR PatternLine:=1 TO 8 DO
      BEGIN
        CurrentLine:=Entry[PatternLine];
        FOR Spot:=8 DOWNTO 1 DO
          BEGIN
            IF Odd(CurrentLine) THEN WriteXY(Spot,PatternLine,#219);
            CurrentLine:=CurrentLine SHR 1
          END
      END
  END;

{ Thu tuc dat cac chu to len man hinh }
PROCEDURE PutBigStr(StartRow :Byte; Message :STRING);
VAR N,Len,StartColumn,
    CurrColumn : Byte;
    St         : STRING[11];
  BEGIN
    St:=Copy(Message,1,10);
    Len:=Length(St);
    StartColumn:=((80-8*Len) DIV 2) AND $00FF;
    FOR n:=1 TO Len DO
      BEGIN
        CurrColumn:=StartColumn+8*(n-1)+1;
        Window(CurrColumn,StartRow,CurrColumn+7,StartRow+8);
        PaintBigChar(Table[Ord(St[n])])
      END;
    Window(1,1,80,25)
  END;


{ Thu tuc tao con chu chay tren man hinh da duoc giai
  thich o tren. Xin ban xem lai cho ro hon }
PROCEDURE CuonChu(VAR St :STRING; Mode :Boolean);
VAR k,Len : Byte;
    Tempo : Char;
  { Thuc hien viec trao doi cac ky tu }
  BEGIN
    Len:=Length(St);
    IF Mode THEN { Chu chay sang trai }
      BEGIN
        Tempo:=St[1];
        FOR k:=1 TO Len-1 DO St[k]:=St[k+1];
        St[Len]:=Tempo
      END
    ELSE
      BEGIN { Chu chay sang phai }
        Tempo:=St[Len];
        FOR k:=Len DOWNTO 2 DO St[k]:=St[k-1];
        St[1]:=Tempo
      END
  END;

{ Xoa vung chua cac ky tu bang mau nen }
PROCEDURE Clear(Row :Byte);
  BEGIN
    Window(1,Row,80,Row+6);
    ClrScr;
    Window(1,1,80,25)
  END;

BEGIN { Bat dau thu tuc Graph_Demo1 }
  TextBackGround(LightMagenta);
  SetBorderColor(Yellow);
  ClrScr;
  St:='TRUONG DAI HOC KY THUAT CONG NGHE ***** ';
  LowSt:='KHOA CONG NGHE THONG TIN  ';
  REPEAT
    SetTextColor(LightCyan,LightMagenta);
    CuonChu(St,Left);
    PutBigStr(5,St);
    SetTextColor(LightGreen,LightMagenta);
    CuonChu(LowSt,Right);
    PutBigStr(15,LowSt);
    GotoXY(80,25);
    Delay(150);
    Clear(5);
    Clear(15)
  UNTIL KeyPressed;
  Readln;
  SetBorderColor(Black) { Thiet lap lai vung bien mac nhien }
END; {Ket thuc PROCEDURE Graph_Demo1}

{ Thu tuc ve Ellipse trong he toa do thuc }
PROCEDURE EllipseBK(Xc,Yc :Integer; Ar,Br,g :Real; Color :Integer);
VAR
  px,py,py1,px1 :Integer;
  x,y,goc,co,
  si,z1,z2,p    :Real;

  BEGIN { Bat dau ve EllipseBK }
    IF g=0 THEN g:=g*0.00001;
    Goc:=(360-g)*pi/180;
    Co:=Cos(goc);
    Si:=Sin(goc);
    x:=0;
    y:=Round(Br);
    z1:=(Ar*Ar)/(Br*Br);
    z2:=(Br*Br)/(Ar*Ar);
    p:=2*z2-2*Br+1;
    WHILE ((z2*x)<=y) DO
       BEGIN
         px:=Round(x*co-y*si);
         py:=Round(x*si+y*co);
         PutPixel(Xc+px,Yc+py,Color);
         PutPixel(Xc+Round(-x*co-y*si),Yc+Round(-x*si+y*co),Color);
         PutPixel(Xc+Round(x*co+y*si),Yc+Round(x*si-y*co),Color);
         PutPixel(Xc-px,Yc-py,Color);
         IF p<0 THEN p:=p+2*z2*(2*x+3)
         ELSE
           BEGIN
             p:=p+2*z2*(2*x+3)+4*(1-y);
             y:=y-1
            END;
        x:=x+1
      END;  {While }
    x:=Round(Ar);y:=0;
    p:=2*z1-2*Ar+1;
    WHILE ((z1*y)<x) DO
       BEGIN
         px:=Round(x*co-y*si);
         py:=Round(x*si+y*co);
         PutPixel(Xc+px,Yc+py,Color);
         PutPixel(Xc+Round(-x*co-y*si),Yc+Round(-x*si+y*co),Color);
         PutPixel(Xc+Round(x*co+y*si),Yc+Round(x*si-y*co),Color);
         PutPixel(Xc-px,Yc-py,Color);
         IF p<0 THEN p:=p+2*z1*(2*y+3)
         ELSE
           BEGIN
             p:=p+2*z1*(2*y+3)+4*(1-x);
             x:=x-1
           END;
          y:=y+1
      END { While }
  END;   { Ket thuc ve EllipseBK }

{ Tao thoi gian tre voi so giay thuc }
PROCEDURE DelayThuc(PSec :Real);
VAR Hour,Min,Sec1,Sec2,
       PHSec1,PHSec2 :Word;
  BEGIN
    GetTime(Hour,Min,Sec1,PHSec1);
    REPEAT
      GetTime(Hour,Min,Sec2,PHSec2)
    UNTIL ((Sec2*100+PHSec2)-(Sec1*100+PHSec1))>=PSec/10
  END;

{ Thu tuc tao ra cac ngoi sao lap lanh }
PROCEDURE Star(NumStar,Colors :Integer);
VAR l,m,i : Integer;
  BEGIN
    Randomize;
    FOR i:=1 TO NumStar DO
    Putpixel(Random(GetMaxX),Random(GetMaxY),Colors)
  END;

{ Thu tuc tao ra sao lon (thuong goi la sao choi) }
PROCEDURE BigStar(X,Y,Colors :Integer);
VAR k : Integer;
  BEGIN
    FOR k:=0 TO 2 DO
      BEGIN
        PutPixel(x+k,y+k,Colors);
        PutPixel(x+k,y-k,Colors);
        PutPixel(x-k,y-k,Colors);
        PutPixel(x-k,y+k,Colors)
      END;
    FOR k:=0 TO 4 DO
      BEGIN
        PutPixel(x-k,y,Colors);
        PutPixel(x+k,y,Colors)
      END;
    FOR k:=0 TO 6 DO
      BEGIN
        PutPixel(x,y-k,Colors);
        PutPixel(x,y+k,Colors)
      END
  END;

{Thu tuc ve vong tron }
PROCEDURE Circles(Xc,Yc,r :Integer; m :Integer);
VAR x,y : Integer;
    p   : Real;

  { Thu tuc xac dinh do dich }
  PROCEDURE Dx;
    BEGIN
      Setcolor(m);
      Line(Xc-x,Yc+y,Xc+x,Yc+y);
      Line(Xc-x,Yc-y,Xc+x,Yc-y);
      Line(Xc-y,Yc+x,Xc+y,Yc+x);
      Line(Xc-y,Yc-x,Xc+y,Yc-x)
    END;

  BEGIN   { Bat dau ve vong tron }
    x:=0;
    y:=r;
    p:=3-2*r;
    WHILE x<y DO
      BEGIN
        Dx;
        IF p<0 THEN p:=p+4*x+6
        ELSE
          BEGIN
            p:=p+4*(x-y)+10;
            y:=y-1
          END;
        x:=x+1
      END;
    IF x=y THEN Dx
  END; { Ket thuc ve vong tron }

{ Thu tuc ve Ellipse quay xung quanh Ellipse noi tren }
PROCEDURE RotateEllipse(Xc,Yc :Integer;Ar,Br,g :Real);
VAR L,M,N,Px,Py,
    Py1,Px1,I,J :Integer;
    X,Y,Co,Si,
    Z1,Z2,P,Goc :Real;

  { Khoi tao trang thai ban dau }
  PROCEDURE DrawOject(x,y :Real);
  VAR Sun,Earth :Integer;
    BEGIN
      Star(3,Random(26));
      Circles(Xc,Yc,25,LightRed);
      EllipseBK(Xc,Yc,Ar,Br,0.1*pi/180,m);
      Circles(Xc+Round(x),Yc+Round(y),10,LightGreen);
      EllipseBK(Xc+Round(x),Yc+Round(y),50,20,g,m);
      DelayThuc(1);
      Star(1000,0);
      Circles(Xc+Round(x),Yc+Round(y),10,n);
      EllipseBK(Xc+Round(x),Yc+Round(y),50,20,g,n)
    END;

  { Cho hien thi sao choi (4 sao choi hien thi truoc) }
  PROCEDURE ShowBigStar(VAR i,j :Integer);
  VAR l :Integer;
    BEGIN
      FOR l:=1 TO 3 DO
      BigStar(Random(GetMaxX)-1,Random(GetMaxX)-1,LightCyan)
    END;

  BEGIN { Bat dau thu tuc RotateEllipse }
    m:=15;
    n:=0;
    Goc:=(360-g)*Pi/180;
    Co:=Cos(goc);
    Si:=Sin(goc);
    Z1:=(Ar*Ar)/(Br*Br);
    Z2:=(Br*Br)/(Ar*Ar);
    Randomize;
    REPEAT
{----- Ve Cung Thu 1 ----}
      x:=0;y:=Br;
      p:=2*z2-2*Br+1;
      ShowBigStar(i,j);
      WHILE (z2*(x/y)<=1) AND (NOT KeyPressed) DO
        BEGIN
          DrawOject(x,y);
          g:=g+1;
          IF p<0 THEN p:=p+2*z2*(2*x+3)
          ELSE
            BEGIN
              p:=p+2*z2*(2*x+3)+4*(1-y);
              y:=y-1
            END;
          x:=x+1
        END; { While }
      p:=2*z1-2*Ar+1;
      BigStar(i,j,0);
      ShowBigStar(i,j);
      WHILE (z1*(y)/(x+1)<1)  AND ((y>=0) OR (x<=Ar)) AND
      (NOT KeyPressed) DO
        BEGIN
          DrawOject(x,y);
          g:=g+1;
          IF p<0 THEN p:=p+2*z1*(2*y+3)
          ELSE
            BEGIN
              p:=p+2*z1*(2*y+3)+4*(1-x);
              x:=x+1
            END;
          y:=y-1
        END; { While }
      BigStar(i,j,0);
{---- Ve Cung Thu 2 ----}
      x:=Ar;y:=0;
      p:=2*z1-2*ar+1;
      ShowBigStar(i,j);
      WHILE (z1*(Abs(y)/Abs(x))<1) AND (NOT KeyPressed) DO
        BEGIN
          DrawOject(x,y);
          g:=g+1;
          IF p<0 THEN p:=p+2*z1*(2*Abs(y)+3)
          ELSE
            BEGIN
              p:=p+2*z1*(2*Abs(y)+3)+4*(1-x);
              x:=x-1
            END;
          y:=y-1
        END;  { While }
      p:=2*z2-2*Br+1;
      BigStar(i,j,0);
      ShowBigStar(i,j);
      WHILE (z2*(Abs(x)/Abs(y-1))<=1) AND ((y>=-Br) OR (x>=0)) AND
      (NOT KeyPressed) DO
        BEGIN
          DrawOject(x,y);
          g:=g+1;
          IF p<0 THEN p:=p+2*z2*(2*Abs(x)+3)
          ELSE
            BEGIN
              p:=p+2*z2*(2*Abs(x)+3)+4*(1-Abs(y));
              y:=y-1
            END;
          x:=x-1
        END; { While }
      BigStar(i,j,0);
{---- Ve Cung Thu 3 ----}
      x:=0;y:=-Br;
      ShowBigStar(i,j);
      WHILE (z2*(Abs(x)/Abs(y))<=1) AND (NOT KeyPressed) DO
        BEGIN
          DrawOject(x,y);
          g:=g+1;
          IF p<0 THEN p:=p+2*z2*(2*Abs(x)+3)
          ELSE
            BEGIN
              p:=p+2*z2*(2*Abs(x)+3)+4*(1-Abs(y));
              y:=y+1
            END;
          x:=x-1
        END; { While }
      p:=2*z1-2*Ar+1;
      BigStar(i,j,0);
      ShowBigStar(i,j);
      WHILE (z1*(Abs(y)/Abs(x+1))<1) AND ((y<=0) OR (x>=-Ar)) AND
      (NOT KeyPressed) DO
        BEGIN
          DrawOject(x,y);
          g:=g+1;
          IF p<0 THEN p:=p+2*z1*(2*Abs(y)+3)
          ELSE
            BEGIN
              p:=p+2*z1*(2*Abs(y)+3)+4*(1-Abs(x));
              x:=x-1
            END;
          y:=y+1
        END;  { While }
      BigStar(i,j,0);
{---- Ve Cung Thu 4 ----}
      x:=-Ar;y:=0;
      p:=2*z1-2*Ar+1;
      ShowBigStar(i,j);
      WHILE (z1*(Abs(y)/Abs(x))<1) AND ((y<=Br) OR (x<=0)) AND
      (NOT KeyPressed)  DO
        BEGIN
          DrawOject(x,y);
          g:=g+1;
          IF p<0 THEN p:=p+2*z1*(2*Abs(y)+3)
          ELSE
            BEGIN
              p:=p+2*z1*(2*Abs(y)+3)+4*(1-Abs(x));
              x:=x+1
            END;
          y:=y+1
        END;  { While }
      p:=2*z2-2*Br+1;
      BigStar(i,j,0);
      ShowBigStar(i,j);
      WHILE (z2*(Abs(x)/Abs(y+1))<=1) AND ((y<=Br) OR (x<=0)) AND
      (NOT KeyPressed) DO
        BEGIN
          DrawOject(x,y);
          g:=g+1;
          IF p<0 THEN p:=p+2*z2*(2*Abs(x)+3)
          ELSE
            BEGIN
              p:=p+2*z2*(2*Abs(x)+3)+4*(1-Abs(y));
              y:=y+1
            END;
          x:=x+1
        END; { While }
      BigStar(i,j,0)
    UNTIL KeyPressed;
    Readln
  END; { Ket thuc thu tuc RotateEllipse }

{ Thu tuc tao ra cac cac hinh Ellipse chuyen dong tren bau troi }
PROCEDURE Graph_Demo2;
  BEGIN { Bat dau thu tuc Graph_demo2 }
    EnterGraph(Mode16);
    RotateEllipse(GetMaxX DIV 2,GetMaxY DIV 2,180,80,60);
    CloseGraph
  END;  { Ket thuc PROCEDURE Graph_Demo2}


{ Dat diem anh ban dau }
PROCEDURE PutTop(x,y :Integer; Color :Byte);
  BEGIN
    PutPixel(x,y+1,Color);
    PutPixel(x,y-1,Color);
    PutPixel(x+1,y,Color);
    PutPixel(x-1,y,Color)
  END;

{ Dat diem anh sau }
PROCEDURE PutBot(x,y :Integer; Color :Byte);
  BEGIN
    PutPixel(x,y-1,Color);
    PutPixel(x,y+1,Color);
    PutPixel(x+1,y-1,Color);
    PutPixel(x+1,y+1,Color);
    PutPixel(x-1,y-1,Color);
    PutPixel(x-1,y+1,Color);
    PutPixel(x+1,y,Color);
    PutPixel(x-1,y,Color)
  END;

{ Thu tuc ve phao bong }
PROCEDURE PunFull(XC,YC,R,R1,R2 :Word; Color1 :Byte);
VAR  X1,Y1,X2,Y2,i,K : Word;
     Color2          : Byte;

  { Kiem tra tinh doi xung }
  PROCEDURE Symmetry;
    BEGIN
      Color2:=Random(Color1);
      PutBot(XC+X1,YC+Y1,Color2);
      PutBot(XC+X1,YC-Y1,Color2);
      PutBot(XC-X1,YC+Y1,Color2);
      PutBot(XC-X1,YC-Y1,Color2);
      PutBot(XC+Y1,YC+X1,Color2);
      PutBot(XC-Y1,YC+X1,Color2);
      PutBot(XC+Y1,YC-X1,Color2);
      PutBot(XC-Y1,YC-X1,Color2)
    END;

  BEGIN { Bat dau kiem tra tinh doi xung }
    FOR i:=1 TO R  DO
      BEGIN
	k:=Round(SQRT(R*R-i*i));
	x1:=i+Random(r1);
	y1:=k+Random(r2);
	Symmetry
      END
    END; { Ket thuc kiem tra }

{ Thu tuc ve phao bong con }
PROCEDURE Pervasive(XC,YC,R,R1,R2,Inc :Word; Color1 :Byte);
VAR  X1,Y1,X2,Y2,i,K :Word;
     Color2          :Byte;

    { Kiem tra tinh doi xung }
    PROCEDURE Symmetry;
      BEGIN
	Color2:=Random(Color1);
	PutTop(XC+X1,YC+Y1,Color2);
	PutTop(XC+X1,YC-Y1,Color2);
	PutTop(XC-X1,YC+Y1,Color2);
	PutTop(XC-X1,YC-Y1,Color2);
	PutTop(XC+Y1,YC+X1,Color2);
	PutTop(XC-Y1,YC+X1,Color2);
	PutTop(XC+Y1,YC-X1,Color2);
	PutTop(XC-Y1,YC-X1,Color2)
      END;

    BEGIN   { Bat dau kiem tra tinh doi xung Module of Pun }
      I:=1;
      REPEAT
	k:=Round(SQRT(R*R-i*i));
	x1:=Random(i)+Random(r1);
	y1:=Random(k)+Random(r2);
	Symmetry;
	i:=i+inc
      UNTIL i> r DIV Round(SQRT(2));
    END; { Ket thuc kiem tra }

  { Thu tuc ve phao bong hoat dong }
  PROCEDURE FireWorks;
  VAR i,j,x,y : Integer;
     BEGIN { Bat dau ve }
       j:=1;
       REPEAT
         i:=1;
         REPEAT
           x:=GetMaxX DIV 2;
           y:=GetMaxY DIV 2;
           i:=i+2;
           Pervasive(x,y,i,10,10,10,MaxColors);
           PunFull(x,y,i,15,15,0);
           Pervasive(x-200,y+200,i MOD 20,10,10,10,MaxColors);
           PunFull(x-200,y+200,i MOD 20,15,15,0);
           Pervasive(x+200,y-200,i MOD 20,10,10,10,MaxColors);
           PunFull(x+200,y-200,i MOD 20,15,15,0);
           Pervasive(x+200,y+200,i MOD 20,10,10,10,MaxColors);
           PunFull(x+200,y+200,i MOD 20,15,15,0);
           Pervasive(x-200,y-200,i MOD 20,10,10,10,MaxColors);
           PunFull(x-200,y-200,i MOD 20,15,15,0);
           IF KeyPressed THEN
             BEGIN
               Readln;
               Exit
             END
         UNTIL i>240
       UNTIL KeyPressed
     END;  { Ket thuc ve }

PROCEDURE Graph_Demo3;
  BEGIN { Bat dau thu tuc Graph_Demo3 }
    EnterGraph(Mode256);
    FireWorks;
    CloseGraph
  END; { Ket thuc PROCEDURE Graph_Demo3}

{ Thu tuc ve cac hinh chu nhat co mau thay doi ngau nhien }
PROCEDURE Graph_Demo4;
VAR Xmax,Ymax : Integer;
    Color     : Word;
  BEGIN  { Bat dau ve }
    EnterGraph(Mode16); { Khoi tao do hoa }
    Randomize;
    WHILE NOT KeyPressed DO
      BEGIN
        Xmax:=GetMaxX;{ Bien giu lai hoanh do lon nhat }
        Ymax:=GetMaxY;{ Bien giu lai tung do lon nhat }
        MoveTo(0,0);  { Dua con tro den toa do dau man hinh }
        Color:=Random(MaxColors)+1;
        WHILE Xmax>1 DO
          BEGIN
            SetColor(Random(Color));
            LineRel(Xmax,0);
            LineRel(0,Ymax);
            Linerel(-Xmax,0);
            LineRel(0,-Ymax);
            Dec(Xmax,2);
            Dec(Ymax,2);
            MoveTo(Succ(GetX),Succ(GetY))
          END
      END;
    CloseGraph;
    Readln
  END; { Ket thuc PROCEDURE Graph_Demo4 }

{ Thu tuc tao hinh KOCH_GON }
PROCEDURE Graph_Demo5;
CONST
   Rads=0.017453293; { De doi Degree sang Radian }
   MaxPoints=100;
   N : Integer=5;    { Bac cua cung KOCH }

TYPE
  TRect=RECORD
          Left,Top,
          Right,Bottom:Integer
        END;
  TRealPoint=RECORD
               x,y:real
             END;
  TRealRect=RECORD
              Left,Top,
              Right,Bottom:Real
            END;
  TRealPolygon=RECORD
                 Num :3..MaxPoints;
                 Points:ARRAY[1..MaxPoints] OF TRealPoint
               END;
VAR
  RealWindow,                   { Toa do thuc }
  NDCViewport     : TRealRect;  { Toa do NDC }
  ScreenWindow    : TRect;      { Man hinh thuc }
  CP              : PointType;  { Chuyen CP tu toa do thuc sang NCD }
  ndcCP,RealCP,                 { CP trong thuc va trong ndc }
  Center          : TRealPoint;
  Sx,Sy,Tx,Ty,CD,               { Cac tham so chuyen doi }
  Radius,G0       : Real;
  A_gon           : TRealPolygon; { So hinh ve }
  Number          : Integer;      { So da giac ve }
  Color           : Word;         { Mau cua da giac }

{ Anh xa 1 RealWindow vao 1 ViewPort NDC }
PROCEDURE SetRect(Left,Top,Right,Bottom :Real; VAR R :TRealRect);
  BEGIN
    r.Left:=Left;
    r.Top:=Top;
    r.Right:=Right;
    r.Bottom:=Bottom
  END;

{ ROUTINE trong HE TOA DO THUC Descartes }
{ Thiet lap 1 vung chu nhat trong he toa do thuc hay NDC }
PROCEDURE MapRect(W,V:TRealRect);
  BEGIN
    sX:=(V.Right-V.Left)/(W.Right-W.Left);
    sY:=(V.Top-V.Bottom)/(W.Top-W.Bottom);
    tX:=(V.Left*W.Right-W.Left*V.Right)/(W.Right-W.Left);
    tY:=(V.Bottom*W.Top-W.Bottom*V.Top)/(W.Top-W.Bottom);
    RealCP.x:=W.Left;
    RealCP.y:=W.Top;
    ndcCP.x:=V.Left;
    ndcCP.y:=V.Top
  END;

{ ROUTINES trong he toa do man hinh :Integer }
PROCEDURE SetScreenWindow (x1,y1,x2,y2 :Integer);
VAR Temp : Integer;
  BEGIN
    IF (x1>=0) AND (y1>=0) AND (x2>=0) AND (y2>=0) AND (x1<x2)
    AND (y1<y2) THEN
      BEGIN
        ScreenWindow.Left:=x1;
        ScreenWindow.Top :=y1;
        ScreenWindow.Right:=x2;
        ScreenWindow.Bottom:=y2;
        WITH ScreenWindow DO CP.x:=ScreenWindow.Left;
        CP.y:=Screenwindow.Top { Xac dinh CP man hinh }
      END
  END;

{ ROUTINES trong he toa do man hinh :Integer }
PROCEDURE SetRealPoint(x,y :Real; VAR aPoint :TRealPoint);
  BEGIN
    aPoint.x:=x;
    aPoint.y:=y
  END;

{ ROUTINES trong he toa do man hinh :Integer }
PROCEDURE Point(x,y : Integer; VAR aPoint : PointType);
BEGIN
   aPoint.x:=x;
   aPoint.y:=y
END;

{ Chuyen 1 diem tu he toa do NDC ra 1
  Form (Left,Top,Width,Height:Integer) }
PROCEDURE NDCToScreenPoint(aNDCPoint:TRealPoint; VAR aPoint:PointType);
VAR Width,Height : Integer;
    Offset       : Real;
  BEGIN
     WITH aNDCPoint DO IF (x>=0)AND (x<=1) AND(y>=0) AND (y<=1) THEN
     BEGIN
       WITH ScreenWindow DO
         BEGIN
           Width:=Right-Left; { Hinh vuong tren man hinh }
           Height:=Bottom-Top;
    	   IF Width>Height THEN Offset:=(Width-Height)/2
	   ELSE Offset:=0;
           IF Width>Height THEN Width:=Height
	   ELSE Height:=Width;
           Point(Round(aNDCPoint.x*Width+Offset)+Left,
           Round(Bottom-aNDCPoint.y*Height),aPoint)
         END
     END
  END;

{ Chuyen 1 diem tu he toa do thuc RealWindow
 (Left,Top,Right,Bottom vao he toa do NDC }
PROCEDURE RealToNDCPoint(RealPoint:TRealPoint; VAR aNDCPoint:TRealPoint);
  BEGIN
    WITH aNDCPoint DO
       BEGIN
         x:=sx*RealPoint.x+tx;
         y:=sy*RealPoint.y+ty
       END
  END;

{ To mau 1 da giac bat ky, khong cat nhau }
PROCEDURE FloodFillReal(RealPoint : TRealPoint; aColor, BoundColor : Word);
VAR
  aPoint   : PointType;
  ndcPoint : TRealPoint;
  BEGIN
    RealToNdcPoint(RealPoint,ndcPoint);
    NDCToScreenPoint(NDCPoint,aPoint);
    SetFillStyle(SolidFill,aColor);
    FloodFill(aPoint.x,aPoint.y,Boundcolor)
  END;

{ Khoi dong che do do hoa va thiet lap moi truong lam viec }
PROCEDURE StartGraphics;
  BEGIN
    EnterGraph(Mode256); { Khoi dong he do hoa 256 mau }
    SetRect(0,1,1,0,RealWindow);     { Khoi tao he toa do thuc }
    SetRect(0,1,1,0,ndcViewport);    { Khoi tao he toa do NDC }
    MapRect(RealWindow,NDCViewport); { Huong hien hanh nam ngang }
    CD:=0;
    SetScreenWindow(0,0,GetMaxX,GetMaxY);
    WITH RealWindow DO SetRealpoint(Left,Top,RealCP);
    WITH ndcViewport DO SetRealpoint(Left,Top,ndcCP)
  END;

{ Dinh toa do cac dinh cua da giac deu NDC tu tam,ban kinh vong
  ngoai tiep, so dinh n, dinh dau tien co phuong la goc g0 tinh
  theo degree }
PROCEDURE MakeNGon(n : Integer; Radius,g0 : Real; Center : TRealPoint;
                   VAR N_gon : TRealPolygon);
VAR
  i : Integer;
  g : Real;
  BEGIN
    IF (n>2) AND (n<=MaxPoints) THEN
      BEGIN
        g:=2*pi/n;             { Goc se quay cho moi dinh }
        g0:=g0*2*pi/360;       { Doi ra radian }
        WITH N_gon DO
          BEGIN
            Num:=n;
            FOR i:=1 TO Num DO { Tinh toa do tung dinh }
              BEGIN
                Points[i].x:=Center.x+Radius*cos(g0);
                Points[i].y:=Center.y+Radius*sin(g0);
                g0:=g0+g
              END
          END
      END
  END;

{ Ve duong thang bang giai thuat Bressenham }
PROCEDURE Bressenham_LINE(x1,y1,x2,y2 :Integer; Color :Word);
VAR
  dX,dY,i,iX,iY,
  x,y,Temp,Incr,
  PlotX,PlotY   : Integer;
  BEGIN
    dX:=x2-x1;dY:=y2-y1;
    iX:= Abs(dX);          { Do thay doi hoanh do }
    iY:= Abs(dY);          { Do thay doi tung do }
    IF iX>iY THEN Incr:=iX { Loop counter theo do thay doi lon}
    ELSE Incr:=iY;
    PlotX:=x1; PlotY:=y1;
    x:=0; y:=0;
    PutPixel(PlotX,PlotY,Color); { Ve diem dau tien }
    i:=0;
    WHILE i<=Incr DO
      BEGIN                  { Xac dinh diem ke tiep se ve }
        x:=x+iX; y:=y+iY;
        IF x>Incr THEN
          BEGIN
            x:=x-Incr;
            IF dX>0 THEN Temp:=1      { x2 0 ben phai cua x1 }
            ELSE IF dX=0 THEN Temp:=0 { x2=x1,duong thang dung }
                 ELSE Temp:=-1;       { x2 o ben trai x1 }
            PlotX:=PlotX+Temp         { Dinh hoanh do diem ke }
          END;
        IF y>Incr THEN
          BEGIN
            y:=y-incr;
            IF dY>0 THEN Temp:=1      { y2 0 ben duoi cua y1 }
            ELSE IF dY=0 THEN Temp:=0 { y2=y1,duong thang ngang}
                 ELSE Temp:=-1;       { y2 o ben tren y1 }
            PlotY:=PlotY+Temp         { Dinh tung do diem ke }
          END;
        PutPixel(PlotX,PlotY,Color);
        Inc(i)
      END
  END;

{ Ve duong thang trong he toa do
  thuc dung thuat giai Bressenham}
PROCEDURE LineToP(aPoint :PointType);
  BEGIN
    Bressenham_Line(CP.x,CP.y,aPoint.x,aPoint.y,getColor);
    CP:=aPoint
  END;

{ Ve tu CP den 1 diem trong NDC }
PROCEDURE LineToNDC(aNDCPoint :TRealPoint);
VAR aPoint : PointType;
  BEGIN
    WITH aNDCPoint DO
    IF (x>=0)AND (x<=1) AND(y>=0) AND (y<=1) THEN
      BEGIN
        NDCToScreenPoint(aNDCPoint,aPoint);
        LineToP(aPoint);
        ndcCP:=aNdcPoint
      END
  END;

{ Ve doan thang tu RealCP toi (x,y) dua CP ve (x,y) }
PROCEDURE LineToReal(x,y :Real);
VAR
  RealP,
  ndcP : TrealPoint;
  BEGIN
    SetRealPoint(x,y,RealP);
    RealtoNDCPoint(RealP,ndcP);
    LinetoNDC(ndcP);
    RealCP:=RealP
  END;

{ Ve doan thang co do dai a trong he thuc }
PROCEDURE LineForwardReal (a :Real);
VAR
  p     : TRealPoint;
  angle : Real;
  BEGIN
    Angle:=2*pi*CD/360; { Doi degree ra radian de tinh sin cos }
    p.x:=RealCP.x+a*Cos(Angle);
    p.y:=RealCP.y+a*Sin(Angle);
    LineToReal(p.x,p.y);
    RealCP:=p
  END;

{ Quay phai Rua 1 goc g don vi do }
PROCEDURE RightTurn(g : Real);
  BEGIN
    CD:=CD-g { Vi phia phai nguoc chieu luong giac }
  END;

{ Quay trai 1 goc g don vi do  }
PROCEDURE LeftTurn (g : Real);
  BEGIN
    CD:=CD+g { Vi phia trai cung chieu luong giac }
  END;

{ Ve Koch cho doan thang AB. Duong cong Koch tai CP,
  goc ban dau CD,degree, co chieu dai len }
PROCEDURE KOCH(Len : Real; n : Integer);
  BEGIN
    IF n<=0 THEN LineForwardReal(Len)
    ELSE
      BEGIN
        Koch(len/3,n-1); LeftTurn(60);
        Koch(len/3,n-1); RightTurn(120);
        Koch(len/3,n-1); LeftTurn(60);
        Koch(len/3,n-1)
      END
  END;

{ Tim goc cua vector di tu p1 den p2, tra tri radian }
FUNCTION GetAngle(p1,p2 : TRealPoint) : Real;
VAR a : Real;
  BEGIN
    IF p2.x<>p1.x THEN
       a:=Arctan((p2.y-p1.y)/(p2.x-p1.x))
    ELSE a:=pi/2;
      IF p2.x<p1.x THEN a:=a+pi;
      IF (a<0) AND (p2.x>p1.x) THEN a:=a+2*pi;
      IF (a=pi/2) AND (p2.y<p1.y) THEN a:=a+pi;
      GetAngle:=a
  END;

{ Dat vi tri con tro tai toa do thuc aPoint.x,aPoint.y }
PROCEDURE MovetoP(aPoint : PointType);
  BEGIN
    Moveto(aPoint.x,aPoint.y);
    CP:=aPoint
  END;

{ Doi CP trong he toa do NDC }
PROCEDURE MoveToNDC(NDC_Point : TRealPoint);
VAR aPoint : PointType;
  BEGIN
     WITH NDC_Point DO
     IF (x>=0)AND (x<=1) AND(y>=0) AND (y<=1) THEN
       BEGIN
         NDCToScreenPoint(NDC_Point,aPoint);
         MovetoP(aPoint);
         ndcCP:=NDC_Point
       END
  END;

{ Doi CP trong he toa do thuc }
PROCEDURE MoveToReal(aRealPoint : TRealPoint);
VAR AndcPoint : TRealPoint;
  BEGIN
    RealCP:=aRealPoint;
    RealToNDCPoint(aRealPoint,AndcPoint);
    MoveToNDC(AndcPoint)
  END;

{ Ve Koch cho doan thang AB }
PROCEDURE KochLine(A,B : TRealPoint; n : Integer);
VAR
  Len : Real;
  BEGIN
    CD:=GetAngle(A,B)/Rads; { Doi ra degree }
    Len:=SQRT((B.y-A.y)*(B.y-A.y)+(B.x-A.x)*(B.x-A.x));
    MovetoReal(A);
    KOCH(Len,n)
  END;

{ Ve KOCH cho 1 da giac }
PROCEDURE KOCHgon(A_gon : TRealPolygon; n : Integer);
VAR i,j : Integer;
  BEGIN
    WITH A_gon DO
    FOR i:=1 TO Num DO
      BEGIN
        j:=i+1;
        IF j>Num THEN j:=1;
        KochLine(Points[i],Points[j],n)
      END
  END;

BEGIN { Bat dau thu tuc Graph_Demo5 }
  StartGraphics;
  SetRect(-0.5,1.5,1.5,-0.5,RealWindow);
  MapRect(RealWindow,ndcViewport);
  WITH RealWindow DO
  SetRealPoint((Left+Right)/2,(Top+Bottom)/2,center);
  Randomize;
  REPEAT
    Number:=Random(MaxColors);
    Radius:=Random;
    g0:=100*Random;
    Color:=Random(MaxColors)+1;
    SetColor(Color);
    MakeNGon(Number,Radius,g0,Center,A_gon);
    KochGon(A_gon,n);
    FloodFillReal(Center,Color,Color);
  UNTIL Keypressed;
  CloseGraph;
  Readln
END;  {Ket thuc PROCEDURE Graph_Demo5}

{ Thu tuc ve con tau vu tru bay tren bau troi day sao }
PROCEDURE PutImagePlay;
VAR
  XX,YY           : ARRAY[1..1000] OF Integer;
  P               : Pointer;
  Angle,Xt,Yt,X0,
  Temp1,Temp2,Y0  : Real;
  X,Y,I,Size      : Integer;

  { Mo phong trang thai dau tien cua tau }
  PROCEDURE MoveSaucer;
    BEGIN
      XT:=Temp1*Cos(Angle)+X0;
      YT:=-Temp2*Sin(Angle)+Y0;
      X:=Round(XT+0.5);
      Y:=Round(YT+0.5);
      PutImage(X,Y,P^,XORPut);
      Delay(100);
      PutImage(X,Y,P^,XORPut);
      Angle:=Angle - Pi / 30;
      IF Angle < PI/2 THEN Angle:=2*Pi + Pi/2
    END; { MoveSaucer }

  { The hien sao lap lanh tren bau troi }
  PROCEDURE RandStart;
  VAR  I,J : Integer;
    BEGIN
      i:=1;
      FOR j:=1 TO 1000 DO
        BEGIN
          PutPixel(XX[i],YY[i],Random(MaxColors));
          i:=i+1;
          IF i>1000 THEN i:=1
        END
    END;

  BEGIN { Than chuong trinh PutImagePlay }
    ClearDevice;
    SetColor(LightCyan); { Draw Saucer }
    Ellipse(100,50,0,360,20,8);
    Ellipse(100,46,190,357,20,6);
    Line(107,44,110,38);
    Circle(110,38,2);
    Line(93,44,90,38);
    Circle(90,38,2);
    SetFillStyle(SolidFill,LightBlue);
    FloodFill(101,54,LightCyan);
    SetFillStyle(SolidFill,LightMagenta);
    FloodFill(94,45,LightCyan);
    Size:=ImageSize(79,36,121,59); { Put size saucer into memory }
    GetMem(P,Size);
    GetImage(79,36,121,59,P^);
    ClearDevice;
    FOR I:=1 TO 1000 DO
      BEGIN { Erase image }
        XX[I]:=Random(GetMaxX);
        YY[I]:=Random(GetMaxY);
        PutPixel(XX[i],YY[i],Random(MaxColors))
      END;
     Angle:=2*Pi+Pi/2;
     X0:=(GetMaxX-42)/2;
     Y0:=(GetMaxY-25)/2;
     Temp1:=X0;
     Temp2:=Y0;
     REPEAT
       MoveSaucer;
       RandStart
     UNTIL KeyPressed;
     FreeMem(P,Size)
  END;  { PutImagePlay }

PROCEDURE Graph_Demo6;
VAR MaxColor : Byte;
  BEGIN { Bat dau thu tuc Graph_Demo6 }
    EnterGraph(Mode16);
    MaxColor:=GetMaxColor;
    PutImagePlay;
    CloseGraph;
    Readln
  END;  { Ket thuc PROCEDURE Graph_Demo6}

{ Thu tuc ve cac hinh thanh co mau thay doi. Mo ta su hoat
  dong cua ham SetPalette }
PROCEDURE PalettePlay;
CONST
  XBars = 15;
  YBars = 10;
VAR
  I, J     : Word;
  X, Y     : Word;
  Color    : Word;         { Mau cua khung }
  ViewInfo : ViewPortType; { Cua so hien tai }
  Width    : Word;         { Do rong cua khung }
  Height   : Word;         { Do cao cua khung }
  OldPal   : PaletteType;  { Nen hien tai }

  BEGIN { Bat dau thu tuc PalettePlay }
    GetPalette(OldPal); { Tra lai bang mau hien hanh }
    GetViewSettings(ViewInfo); { Tra lai tam nhin va cac tham so }
    WITH ViewInfo DO
      BEGIN
        Width := (x2-x1) DIV XBars;
        Height := (y2-y1) DIV YBars
      END;
    X := 0; Y := 0;
    Color := 0;
    FOR J := 1 TO YBars DO
    BEGIN
      FOR I := 1 TO XBars DO
        BEGIN
          SetFillStyle(SolidFill, Color);
          Bar(X, Y, X+Width, Y+Height);
          Inc(X, Width+1);
          Inc(Color);
          Color := Color MOD (MaxColors+1)
        END;
      X := 0;
      Inc(Y, Height+1)
    END;
    REPEAT
      SetPalette(Random(MaxColors + 1), Random(65))
    UNTIL KeyPressed;
    SetAllPalette(OldPal)
  END; { Ket thuc thu tuc PalettePlay }

PROCEDURE Graph_Demo7;
VAR MaxColor :Byte;
  BEGIN { Bat dau thu tuc Graph_demo7 }
    EnterGraph(Mode16);
    MaxColor:=GetMaxColor;
    PalettePlay;
    CloseGraph;
    Readln
  END;  { Ket thuc PROCEDURE Graph_Demo7 }

{ Thu tuc mo phong con doi dang bay trong ban dem
  <ESC> : Exit, <C> and <B> : Change Color }
PROCEDURE Graph_Demo8;
CONST
  Memory = 100;
  Windows = 4;
TYPE
  ResolutionPreference=(Lower, Higher);
  ColorList=ARRAY[1..Windows] OF Integer;
  ListParts=RECORD
              LX1,LY1,
              LX2,LY2 : Integer;
              LColor  : ColorList
            END;
VAR
  Xmax,Ymax,ViewXmax, { Cac toa do khung nhin, duong, Mode man hinh }
  ViewYmax,X1,X2,Y1,Y2,
  CurrentLine,ColorCount,
  IncrementCount,DeltaX1,
  DeltaY1,DeltaX2,DeltaY2,
  BackColor,MaxDelta : Integer;   { So duong thang de ve con doi }
  Line               : ARRAY[1..Memory] OF ListParts;
  Colors             : ColorList; { Danh sach mau }
  Ch                 : Char;      { Doi doc ky tu de xu ly }
  MaxColors          : Word;      { Luu gia tri mau lon nhat co the }
  ChangeColors       : Boolean;   { Thay doi mau nen va mau con doi }

{ Dong thong bao cuoi man hinh }
PROCEDURE MessageFrame(Msg : STRING);
  BEGIN
    SetColor(MaxColors);
    OutTextXY((Xmax DIV 2)-140, Ymax-(TextHeight('M')+2), Msg);
    { Go back to the main window }
  END;{ MessageFrame }

{ Cho y kien nguoi dung }
PROCEDURE WaitToGo;
VAR Ch : Char;
  BEGIN
    MessageFrame('Press a key to stop action, Esc quits.');
    REPEAT UNTIL KeyPressed;
    Ch := ReadKey;
    IF Ch = #27 THEN EXIT
    ELSE ClearViewPort;
    MessageFrame('Press a key to stop action, Esc quits.');
  END; { WaitToGo }

{ Cai dat cac tham so cho chuong trinh ban dau }
PROCEDURE StartProgram;
VAR
  Err,StartX,
  StartY,I   : Integer;
  Resolution : ResolutionPreference;
  S          : STRING;
  BEGIN
    Resolution:=Lower;
    IF Paramcount>0 then
      BEGIN
        S := Paramstr(1);
        IF s[1] = '/' THEN
        IF UpCase(S[2]) = 'H' THEN
        Resolution := Higher
      END;
    CurrentLine    := 1;
    ColorCount     := 0;
    IncrementCount := 0;
    Ch:= ' ';
    EnterGraph(Mode16);{ Khoi tao che do do hoa }
    MaxDelta:=16;  { Khoi tao gia tri cho man hinh EGAVGA }
    MaxColors := GetMaxColor;
    BackColor := 0;
    ChangeColors := TRUE;
    Xmax := GetMaxX;
    Ymax := GetMaxY;
    ViewXmax := Xmax-2;
    ViewYmax := (Ymax-(TextHeight('M')+4)-1)-2;
    StartX := Xmax DIV 2;
    StartY := Ymax DIV 2;
    FOR I := 1 TO Memory DO
      WITH Line[I] DO
        BEGIN
          LX1 := StartX;
          LX2 := StartX;
          LY1 := StartY;
          LY2 := StartY
        END;
    X1 := StartX;
    X2 := StartX;
    Y1 := StartY;
    Y2 := StartY
  END; { StartProgram }

{ Trao doi truc X }
PROCEDURE AdjustX(VAR X, DeltaX : Integer);
VAR TestX : Integer;
  BEGIN
    TestX := X + DeltaX;
    IF (TestX < 1) OR (TestX > ViewXmax) THEN
      BEGIN
        TestX := X;
        DeltaX := -DeltaX
      END;
    X := TestX
  END;

{ Trao doi truc Y }
PROCEDURE AdjustY(VAR Y,DeltaY: integer);
VAR TestY : Integer;
  BEGIN
    TestY := Y + DeltaY;
    IF (TestY < 1) OR (TestY > ViewYmax) THEN
      BEGIN
        TestY := Y;
        DeltaY := -DeltaY
      END;
    Y := TestY
  END;

{ Su dung cac mau hinh ngau nhien }
PROCEDURE SelectNewColors;
  BEGIN
    IF NOT ChangeColors THEN Exit; { Neu khong chon mau thi thoat }
    Colors[1]  := Random(MaxColors)+1;
    Colors[2]  := Random(MaxColors)+1;
    Colors[3]  := Random(MaxColors)+1;
    Colors[4]  := Random(MaxColors)+1;
    ColorCount := 3*(1+Random(5))
  END;

{ Su dung cac tri ngau nhien }
PROCEDURE SelectNewDeltaValues;
  BEGIN
    DeltaX1 := Random(MaxDelta)-(MaxDelta DIV 2);
    DeltaX2 := Random(MaxDelta)-(MaxDelta DIV 2);
    DeltaY1 := Random(MaxDelta)-(MaxDelta DIV 2);
    DeltaY2 := Random(MaxDelta)-(MaxDelta DIV 2);
    IncrementCount := 2*(1+Random(4))
  END;

{ Luu duong thang hien tai }
PROCEDURE SaveCurrentLine(CurrentColors: ColorList);
  BEGIN
    WITH Line[CurrentLine] DO
      BEGIN
        LX1 := X1;
        LY1 := Y1;
        LX2 := X2;
        LY2 := Y2;
        LColor := CurrentColors
      END
  END;

{ Thu tuc ve cac doan thang }
PROCEDURE Draw(x1,y1,x2,y2,Color: Word);
  BEGIN
    SetColor(Color);
    Graph.Line(x1,y1,x2,y2)
  END;

{ Ve cac hinh ban dau }
PROCEDURE Regenerate;
VAR I : integer;
  BEGIN
    FOR I := 1 TO Memory DO
      WITH Line[I] DO BEGIN
        Draw(LX1,LY1,LX2,LY2,LColor[1]);
        Draw(ViewXmax-LX1,LY1,ViewXmax-LX2,LY2,LColor[2]);
        Draw(LX1,ViewYmax-LY1,LX2,ViewYmax-LY2,LColor[3]);
        Draw(ViewXmax-LX1,ViewYmax-LY1,ViewXmax-LX2,ViewYmax-LY2,LColor[4])
      END;
    WaitToGo
  END;

{ Cap nhat lai cac duong da ve }
PROCEDURE Updateline;
  BEGIN
    Inc(CurrentLine);
    IF CurrentLine > Memory THEN CurrentLine := 1;
    Dec(ColorCount);
    Dec(IncrementCount)
  END;

{ Thay doi mau nen va mau doan thang khi bam phim B va C }
PROCEDURE CheckForUserInput;
  BEGIN
    IF KeyPressed THEN
      BEGIN
        Ch := ReadKey;
        IF Upcase(Ch) = 'B' THEN
          BEGIN
            IF BackColor > MaxColors THEN
               BackColor := 0
            ELSE Inc(BackColor);
            SetBkColor(BackColor);
          END
        ELSE  { Thay doi mau hinh khi bam phim C }
          IF Upcase(Ch) = 'C' THEN
            BEGIN
              IF ChangeColors THEN
                 ChangeColors := FALSE
              ELSE ChangeColors := TRUE;
              ColorCount := 0
            END
          ELSE IF Ch<>#27 THEN Regenerate
      END
  END;

{ Ve cac duong hien tai }
PROCEDURE DrawCurrentLine;
VAR C1,C2,C3,C4 : Integer;
  BEGIN
    { Save trang thai ban dau }
    C1 := Colors[1];
    C2 := Colors[2];
    C3 := Colors[3];
    C4 := Colors[4];
    IF MaxColors = 1 THEN
      BEGIN
        C2 := C1;
        C3 := C1;
        C4 := C1
      END;
    Draw(X1,Y1,X2,Y2,C1);
    Draw(ViewXmax-X1,Y1,ViewXmax-X2,Y2,C2);
    Draw(X1,ViewYmax-Y1,X2,ViewYmax-Y2,c3);
    IF MaxColors = 3 THEN c4 := Random(3)+1; { Alternate colors }
    Draw(ViewXmax-X1,ViewYmax-Y1,ViewXmax-X2,ViewYmax-Y2,c4);
    SaveCurrentLine(Colors)
  END;

PROCEDURE EraseCurrentLine;
  BEGIN
    WITH Line[CurrentLine] DO
      BEGIN
        Draw(LX1,LY1,LX2,LY2,0);
        Draw(ViewXmax-LX1,LY1,ViewXmax-LX2,LY2,0);
        Draw(LX1,ViewYmax-LY1,LX2,ViewYmax-LY2,0);
        Draw(ViewXmax-LX1,ViewYmax-LY1,ViewXmax-LX2,ViewYmax-LY2,0)
      END
  END;

{ Ve hinh chyen dong ngau nhien }
PROCEDURE DoArt;
  BEGIN
    SelectNewColors;
    REPEAT
      EraseCurrentLine;
      IF ColorCount = 0 THEN SelectNewColors;
      IF IncrementCount=0 THEN SelectNewDeltaValues;
      AdjustX(X1,DeltaX1); AdjustX(X2,DeltaX2);
      AdjustY(Y1,DeltaY1); AdjustY(Y2,DeltaY2);
      IF Random(5)=3 THEN
        BEGIN
          x1 := (x1+x2) DIV 2; { Shorten the lines }
          y2 := (y1+y2) DIV 2
        END;
      DrawCurrentLine;
      Updateline;
      CheckForUserInput
    UNTIL Ch=#27
  END;

BEGIN { Bat dau thu tuc Graph_Demo8 }
   StartProgram;  { Khoi tao cac tham so }
   MessageFrame('Press a key to stop action, Esc quits.');
   DoArt;         { Ve hinh ngau nhien }
   CloseGraph;    { Dong che do do hoa }
   RestoreCrtMode { Quay ve che do text }
END; { Ket thuc PROCEDURE Graph_Demo8 }

{ Cai tham so cho cua so }
PROCEDURE Windows(F1,F2,F3,F4:Real);
  BEGIN
    XgFen:=F1; {Hoanh do trai}
    XdFen:=F2; {Hoanh do phai}
    YbFen:=F3; {Tung do duoi}
    YhFen:=F4  {Tung do tren}
  END;

{ Cai tham so cho tam nhin }
PROCEDURE ViewPort(C1,C2,C3,C4:Integer);
  BEGIN             {Co doi chieu truc Y}
    XgClot:=C1;     {Hoanh do trai}
    XdClot:=C2;     {Hoanh do phai}
    YbClot:=C3;     {Tung do duoi}
    YhClot:=C4;     {Tung do tren}
    Xtl:=(XdClot-XgClot)/(XdFen-XgFen);   { Ti le cua so }
    Ytl:=(YhClot-YbClot)/(YhFen-YbFen);   { Tam nhin }
    { Khoi tao ViewPort theo tam nhin va cua so }
    SetViewPort(XgClot,GetMaxY-YhClot,XdClot,GetMaxY-YbClot,ClipOn)
  END;

{ Dung thuat toan CONHEN-SUTHERLAND de cut }
PROCEDURE Cuts(X1,Y1,X2,Y2 : Real);
TYPE Region=(Left,Right,Low,High);
     Code = SET OF Region;
VAR
  C,C1,C2  : Code;
  X,Y      : Real;
  XX1,YY1,
  XX2,YY2  : Integer;

  { Tao ma nhi phan de xu ly cac bit }
  PROCEDURE CodeBinary(X,Y:Real; VAR C:Code);
    BEGIN
      C:=[];
      IF X < XgFen THEN C:=[Left]
      ELSE IF X > XdFen THEN C:=[Right];
      IF Y < YbFen THEN C:=C+[Low]
      ELSE IF Y > YhFen THEN C:=C+[High]
    END;   { Code Binary }
  { Bat dau cuts }
  BEGIN
    CodeBinary(X1,Y1,C1);      { Tao ma thuoc tinh }
    CodeBinary(X2,Y2,C2);      { Cua hai diem dau }
    WHILE (C1 <> []) OR (C2 <> []) DO { C1 va C2 la rong }
      BEGIN   { Cat dan de tim doan hien thi }
        IF (C1*C2)<>[] THEN Exit;   { Cat toan bo }
          IF C1=[] THEN C:=C2
          ELSE C:=C1;
          IF Left IN C THEN
            BEGIN
              X:=XgFen;
              Y:=Y1+(Y2-Y1)*(XgFen-X1)/(X2-X1)
            END
          ELSE
            IF Right IN C THEN
              BEGIN
                X:=XdFen;
                Y:=Y1+(Y2-Y1)*(XdFen-X1)/(X2-X1)
              END
            ELSE
              IF Low IN C THEN
                BEGIN
                  Y:=YbFen;
                  X:=X1+(X2-X1)*(YbFen-Y1)/(Y2-Y1)
                END
              ELSE
                IF High IN C THEN
                  BEGIN
                    Y:=YhFen;
                    X:=X1+(X2-X1)*(YhFen-Y1)/(Y2-Y1)
                  END;
        IF C=C1 THEN { X1,Y1 nam ngoai cua so }
          BEGIN
            X1:=X;
            Y1:=Y;
            CodeBinary(X,Y,C1)
          END
        ELSE
          BEGIN   { X2,Y2 nam ngoai cua so }
            X2:=X;
            Y2:=Y;
            CodeBinary(X,Y,C2)
          END
      END;    { While }
    XX1:=Round((X1-XgFen)*Xtl);
    YY1:=Round((YhFen-Y1)*Ytl);
    XX2:=Round((X2-XgFen)*Xtl);
    YY2:=Round((YhFen-Y2)*Ytl);
    MoveTo(XX1,YY1);
    LineTo(XX2,YY2)
  END;  { Cat }

{ Ve den toa do X,Y}
PROCEDURE LineToReal(X,Y:Real);
  BEGIN
    XP2:=X;
    YP2:=Y;
    Cuts(XP1,YP1,XP2,YP2);
    XP1:=XP2;
    YP1:=YP2
  END;

{ Dua CP den toa do X,Y}
PROCEDURE MoveToReal(X,Y : Real);
  BEGIN
    XP1:=X;
    YP1:=Y;
    LineToReal(X,Y)
  END;

{ Ve vien xung quanh khung nhinh }
PROCEDURE PaintBox(Color : Byte);
  BEGIN
    SetColor(Color);
    MoveToReal(XgFen,YbFen);
    LineToReal(XdFen,YbFen);
    LineToReal(XdFen,YhFen);
    LineToReal(XgFen,YhFen);
    LineToReal(XgFen,YbFen)
  END;

{ Cho go mot phim de thoat chuong trinh }
PROCEDURE Attente;
VAR Ch : Char;
  BEGIN
    Sound(400);
    Delay(200);
    NoSound;
    REPEAT UNTIL Keypressed;
    Ch:=ReadKey
  END;

{ Bat dau ve hinh }
PROCEDURE PaintBolygone(Ordre,Pas :Integer);
VAR ADeg,ARad : Real;
  BEGIN                   { Ve  Bolygone }
    ADeg := 0;            { Goc tinh bang do }
    REPEAT
      ARad:=Pi*ADeg/180;  { Doi do sang Radians }
      MoveToReal(Cos(ARad),Sin(ARad));
      LineToReal(Cos(Ordre*Arad),Sin(Ordre*Arad));
      ADeg:=ADeg+Pas
    UNTIL ADeg>360
  END;

{ Chyuong trinh ve hinh Bolygone trong he toa do Descart2D }
PROCEDURE Graph_Demo9;
VAR Ordre,Pas : Integer;
  BEGIN                            { Bat dau thu tuc Graph_Demo9 }
    Ordre:=11;                     { Bac cua Bolygon }
    Pas:=1;                        { Gia so goc }
    EnterGraph(Mode16);{ Khoi dong che do doa hoa }
    Windows(-1.02,1.02,-1.02,1.02);{ Khoi tao cua so lam viec }
    ViewPort(100,540,10,450);      { Khoi tao tam nhin cua man hinh CGA }
    PaintBox(Yellow);              { Ve vien xung quanh hinh ve }
    SetColor(LightGreen);          { Thiet lap mau cua hinh ve }
    PaintBolygone(Ordre,Pas);      { Ve hinh Bolygone }
    Attente;                       { Cho go phim }
    CloseGraph                     { Dong che do do hoa }
  END;                             { Ket thu thu tuc Graph_demo9 }

PROCEDURE Graph_Demo10;
CONST
  Lim=3;
  Dong=1;

{ Khoi tao mang co 5 phan tu de chua cac canh cua tam giac }
TYPE Table=ARRAY[1..5] OF Real;

VAR
  NbreTri,DivCote,Repet : Byte;
  X,Y                   : Table;

{ Ve mot da giac bat ky }
PROCEDURE MultiAngle(X,Y : Table; Lim : Integer; Mode : Integer);
VAR I:Integer;
  BEGIN
    MoveToReal(X[1],Y[1]);
    FOR I := 2 TO Lim DO LineToReal(X[I],Y[I]);
    IF Mode = Dong THEN LineToReal(X[1],Y[1]);
    { Luu Y : Net cuoi phai trung voi net dau }
  END;

PROCEDURE DrawMultiAngle(X,Y : Table; Lim : Integer; Mode : Integer);
  BEGIN     { Ve da giac }
    SetColor(Yellow);
    MultiAngle(X,Y,Lim,Mode)
  END;

{ Cai dat cac tham so can thiet }
PROCEDURE InstallParameters;
  BEGIN
    { So duong, diem chia, lan lap }
    NbreTri:=12; Divcote:=12; Repet:=18
  END;

{ Khoi ta cac tri de ve hinh }
PROCEDURE DrawOject(NbreTri,Dcot,Rep :Byte);
VAR
  Angle,Coef : Real;
  NumTri,i,j : Byte;
  BEGIN
    Coef := 2*Pi/NbreTri; { Gia so goc }
    FOR NumTri := 1 TO NbreTri DO
      BEGIN
        X[1]  := 0;Y[1]  := 0;    { Cac toa do cua tam giac }
        Angle := (NumTri-1)*Coef; { Xac dinh goa cua tam giac }
        X[2]  := Cos(Angle);Y[2]  := Sin(Angle); { Goc cua da giac }
        Angle := NumTri*Coef;X[3] := Cos(Angle);
        Y[3]  := Sin(Angle);
        X[4]  := 0;Y[4]  := 0;
        FOR i := 1 TO Rep DO
          BEGIN
            DrawMultiAngle(X,Y,Lim,Dong);
            IF Odd(NumTri) THEN { Tam giac le }
              BEGIN  { Quay nguoc chieu kim dong ho }
                FOR j := Lim+1 DOWNTO 2 DO
                  BEGIN
                    X[j] := X[j]+(X[j-1]-X[j])/Dcot;
                    Y[j] := Y[j]+(Y[j-1]-Y[j])/Dcot
                  END;
                X[1] := X[Lim+1];
                Y[1] := Y[Lim+1]
              END
            ELSE     { Tam giac chan }
              BEGIN  { Quay cung chieu kim dong ho }
                FOR j := 1 TO Lim DO
                  BEGIN
                    X[j] := X[j]+(X[j+1]-X[j])/Dcot;
                    Y[j] := Y[j]+(Y[j+1]-Y[j])/Dcot
                  END;
                X[Lim+1] := X[1];
                Y[Lim+1] := Y[1]
              END { If }
          END { For }
      END { For NumTri }
  END; { Ve }

BEGIN { Bat dau thu tuc Graph_Demo10 }
  InstallParameters;               { Nhap so lieu cac thiet }
  EnterGraph(Mode16);  { Khoi tao che do do hoa }
  Windows(-1.02,1.02,-1.02,1.02);  { Khoi tao cua so lam viec }
  ViewPort(100,540,10,450);        { Khoi tao tam nhin ung voi cua so }
  PaintBox(Yellow);                { Ve vien xung quyanh hinh ve }
  DrawOject(NbreTri,DivCote,Repet);{ Ve hinh khinh van hoa }
  Attente;                         { Cho go phim }
  CloseGraph                       { Dong che do do hoa }
END; { Ket thuc PROCEDURE Graph_Demo10 }

PROCEDURE Graph_Demo11;
CONST
  LimX=640; { Dimensions de la carte EGAVGA }
  LimY=330; { Avec place pour une ligne de text }

TYPE Table=ARRAY[0..LimX] OF Integer;
     GenreDeProjection = (Perspective,Parallele);
     { Phep chieu phoi canh va song song }
VAR
  HMax,HMin,TabAux           : Table; { Toa do cua cac mang }
  Vue                        : Char;  { Bien chon kieu phep chieu }
  Projection                 : GenreDeProjection; { Phep chieu }
  C1,C2,C3,C4,Xg,Yg,Xd,Yd,   { Toa do cua so, duong va so diem }
  NbreLignes,NbrePoints      : Integer;
  X1,X2,Y1,Y2,IncX,IncY,F1,  { Toa do tam nhin, ty le, phep chieu }
  F2,F3,F4,EchX,EchY,Ech,DE,
  Theta,Phi,Aux1,Aux2,Aux3,
  Aux4,Aux5,Aux6,Aux7,Aux8,
  XObs,YObs,ZObs,Xproj,Yproj : Real;

{ Ham bieu thi hinh nam bao mua }
FUNCTION F(X,Y : Real) : Real;
  BEGIN
    F:=10*Sin(SQRT(SQR(x)+SQR(y)))/SQRT(SQR(x)+SQR(y))
  END;

{ Ham xet dau }
FUNCTION Signe (X : Real) : Integer;
  BEGIN
    IF X > 0 THEN
       Signe:=1
    ELSE
       IF X < 0 THEN
          Signe:=-1
       ELSE Signe:=0
END;

{ Khoi tao cac tham so can thiet }
PROCEDURE EntreeDesParametresDevision;
  BEGIN
    X1:=-8.2; X2:=8.2;    { Toa do theo truc X }
    Y1:=-8.2; Y2:=8.2;    { Toa do theo truc Y }
    NbreLignes:=57; NbrePoints:=83;{ Duong va so diem tren duong }
    DE:=1; Projection:=Parallele;  { Thuc hien phep chieu song song }
    Vue:='R'; Theta:=45; Phi:=20   { Dinh cac goc ban dau }
  END;

{ Khoi tao cac tham so cho chuong trinh }
PROCEDURE InitialisationsDiverses;
VAR
  Aux : Real;
  i   : Integer;
  BEGIN
    IncX:=(x2-x1)/NbrePoints;      { Tinh ty le giua cac khoang chia }
    IncY:=(y2-y1)/NbreLignes;      { Tinh ty le giau cac duong ve }
    C1:=70;C2:=710;C3:=57;C4:=145; { Thiet lap tam nhin (man hinh EGAVGA }
    F1:=1E37;F2:=-1E37;
    F3:=1E37;F4:=-1E37;            { Thiet lap cua so theo tam hinh }
    Xg:=-1;Yg:=-1;Xd:=-1;Yd:=-1;   { Do dich }
    Fillchar (HMax,Sizeof(HMax),0);
    FOR i:= 0 TO LimX DO HMin[i]:=LimY
  END;

{Khoi tao cac tri cho phep chieu}
PROCEDURE InitialiseProjection;
VAR Th, Ph : Real; { Theta,Phi }
  BEGIN
    Th   := Pi*Theta/180; { Doi do ra Radian }
    Ph   := Pi*Phi/180;
    Aux1 := Sin(Th);   Aux2 := Sin(Ph);
    Aux3 := Cos(Th);   Aux4 := Cos(Ph);
    Aux5 := Aux3*Aux2; Aux6 := Aux1*Aux2;
    Aux7 := Aux3*Aux4; Aux8 := Aux1*Aux4
  END;

{ Khoi tao cac phep chieu }
PROCEDURE Projette(X,Y,Z : Real);
  BEGIN
    XObs := -X*Aux1+Y*Aux3;
    YObs := -X*Aux5-Y*Aux6+Z*Aux4;
    IF Projection = Perspective THEN
      BEGIN
        ZObs  := -X*Aux7-Y*Aux8-Z*Aux2;
        XProj := DE*XObs/ZObs;
        YProj := DE*YObs/ZObs
      END
    ELSE
      BEGIN
        XProj := DE*XObs;
        YProj := DE*YObs
      END
  END;

{ Tim cua so hien thi hinh anh }
PROCEDURE RechercheFenetre;
VAR
  X,Y,Z       : Real;    { Toa do cua ham }
  Ligne,Point : Integer; { Duong, so diem tren duong }
 BEGIN
   FOR Ligne:=0 TO NbreLignes DO
     BEGIN
       Y:=Y2-Ligne*IncY;
       FOR Point:=0 TO NbrePoints DO
         BEGIN
           X:=X1+Point*IncX;
           Z:=F(X,Y);
           Projette(X,Y,Z);
           IF XProj < F1 THEN F1:=XProj;
           IF XProj > F2 THEN F2:=XProj;
           IF XProj < F3 THEN F3:=YProj;
           IF XProj > F4 THEN F4:=YProj
         END { For Point }
     END { For Ligne }
  END;

{ Tinh ty le}
PROCEDURE CalculeEchelles;
  BEGIN
    EchX:=(C2-C1)/(F2-F1); { Ty le theo truc X }
    EchY:=(C4-C3)/(F4-F3); { Ty le theo truc Y }
    EchX:=EchY
  END;

PROCEDURE Horizon (X1,Y1,X2,Y2 : Integer);
VAR
  X,Y,dX : Integer;
  Pente  : Real;   { He so goc }

    FUNCTION Max (X1,X2:Integer):Integer;
      BEGIN
        IF X1 > X2 THEN Max:=X1
        ELSE Max:=X2
      END;

    FUNCTION Min(X1,X2 : Integer) : Integer;
      BEGIN
        IF X1 < X2 THEN Min:=X1
        ELSE Min:=X2
      END;
  BEGIN
    dX:=Signe(X2-X1);
    IF dX=0 THEN
      BEGIN
        HMax[X2+1]:=Max(HMax[X2],Y2);
        HMin[X2+1]:=Min(HMin[X2],Y2)
      END
    ELSE
      BEGIN
        Pente:=(Y2-Y1)/(X2-X1);
        FOR X:=X2+1 TO X1 DO
          BEGIN
            Y:=Round(Pente*(X-X1)+Y1);
            HMax[X]:=Max(HMax[X],Y);
            HMin[X]:=Min(HMin[X],Y);
          END { For X}
      END { Else }
  END; { Horizon }

{ Cac truong hop kha kien }
PROCEDURE Visibilite(X,Y : Integer; VAR Visi : ShortInt);
  BEGIN
    IF (Y < HMax[X]) AND (Y > HMin[X]) THEN Visi:=0
    ELSE IF Y >= HMax[X] THEN  Visi:=1
         ELSE Visi:=-1
  END;

PROCEDURE Inter(X1,Y1,X2,Y2:LongInt; VAR TabAux:Table; VAR Xi,Yi:Integer);
VAR Den,Xii,Yii : Real;
  BEGIN
    IF X2-X1=0 THEN
      BEGIN
        Xii:=X2;
        Yii:=TabAux[X2]
      END
    ELSE
      BEGIN
        Den:=Y2-Y1-TabAux[X2]+TabAux[X1];
        IF Den <> 0 THEN
          BEGIN
            Xii:=(X1*(Y2-TabAux[X2])+X2*(TabAux[X1]-Y1))/Den;
            Yii:=(Y2*TabAux[X1]-Y1*TabAux[X2])/Den
          END
        ELSE
          BEGIN
            Xii:=X2;
            Yii:=Y2
          END
      END;
    Xi:=Round(Xii);
    Yi:=Round(Yii)
  END;

{ Xac dinh duong bien cho hai ho duong cong }
PROCEDURE AretesDeFermeture(X,Y : Integer; VAR XLateral,YLateral:Integer);
  BEGIN
    IF XLateral <> -1 THEN Horizon(XLateral,YLateral,X,Y);
    XLateral:=X;
    YLateral:=Y
  END;

{ Ve cac ham }
PROCEDURE Dessinefonction;
VAR
  Xe,Ye,Ligne,Point,
  XPrec,YPrec,XCour,             { Duong, so diem, Prec }
  YCour,Xi,Yi        : Integer;
  VisiCour, VisiPrec : Shortint; { Cac ho Visi }
  X,Y,Z              : Real;     { Toa do cac ham }

  BEGIN
    FOR Ligne := 0 TO NbreLignes DO
      BEGIN
        SetColor(LightCyan);
        Y:=Y2-Ligne*IncY;
        X:=X1;
        Z:=F(X,Y);
        Projette (X,Y,Z);
        XPrec:=Round((XProj-F1)*EchX)+C1;
        YPrec:=Round((YProj-F3)*EchY)+C3;
        AretesDeFermeture (XPrec,YPrec,Xd,Yd);
        MoveTo(XPrec,LimY-YPrec);
        Visibilite(XPrec,YPrec,VisiPrec);
        FOR Point:=0 TO NbrePoints DO
          BEGIN
            X:=X1+Point*IncX;
            Z:=F(X,Y);
            Projette (X,Y,Z);
            XCour:=Round((XProj-F1)*EchX)+C1;
            YCour:=Round((YProj-F3)*EchY)+C3;
            Visibilite(XCour,YCour,VisiCour);
            IF (HMax[XCour]=0) OR (HMin[XCour]=LimY) THEN
	    VIsiCour:=VisiPrec;
              IF VisiCour=VisiPrec THEN
                BEGIN
                  IF (VisiCour=1) OR (VisiCour=-1) THEN
                    BEGIN
                      Line(XPrec,LimY-YPrec,XCour,LimY-YCour);
                      Horizon(XPrec,YPrec,XCour,YCour)
                    END { VisiCour=1 OR VisiCour=-1 }
                END {VisiCour=VisiPrec }
              ELSE
                BEGIN
                  IF VisiCour = 0 THEN
                    BEGIN
                      IF VisiPrec = 1 THEN
                      Inter(XPrec,YPrec,XCour,YCour,HMax,Xi,Yi)
                      ELSE
                        Inter(XPrec,YPrec,XCour,YCour,HMin,Xi,Yi);
                        Line(XPrec,LimY-YPrec,Xi,LimY-Yi);
                        Horizon(XPrec,YPrec,Xi,Yi);
                    END { VisiCour=0 }
                  ELSE
                    BEGIN
                      IF VisiCour=1 THEN
                        BEGIN
                          IF VisiPrec=0 THEN
                            BEGIN
                              Inter(XPrec,YPrec,XCour,YCour,HMax,Xi,Yi);
                              Line(Xi,LimY-Yi,XCour,LimY-YCour);
                              Horizon(Xi,Yi,XCour,YCour)
                            END { VisiPrec=0 }
                          ELSE
                            BEGIN
                              Inter(XPrec,YPrec,XCour,YCour,HMin,Xi,Yi);
                              Line(XPrec,LimY-YPrec,Xi,LimY-Yi);
                              Horizon(XPrec,YPrec,Xi,Yi);
                              Inter(XPrec,YPrec,XCour,YCour,HMax,Xi,Yi);
                              Line(Xi,LimY-Yi,XCour,LimY-YCour);
                              Horizon(Xi,Yi,XCour,YCour)
                            END { Else of VisiPrec=0 }
                        END { VisiCour=1 }
                      ELSE
                        BEGIN
                          IF VisiPrec = 0 THEN
                            BEGIN
                              Inter(XPrec,YPrec,XCour,YCour,HMin,Xi,Yi);
                              Line(Xi,LimY-Yi,XCour,LimY-YCour);
                              Horizon(Xi,Yi,XCour,YCour)
                            END { VisiPrec=0 }
                          ELSE
                            BEGIN
                              Inter(XPrec,YPrec,XCour,YCour,HMax,Xi,Yi);
                              Line(XPrec,LimY-YPrec,Xi,LimY-Yi);
                              Horizon(XPrec,YPrec,Xi,Yi);
                              Inter(XPrec,YPrec,XCour,YCour,HMin,Xi,Yi);
                              Line(Xi,LimY-Yi,XCour,LimY-YCour);
                              Horizon(Xi,Yi,XCour,YCour)
                            END { Else of VisiPrec=0 }
                        END { Else of VisiCour=1 }
                    END { Else of VisiCour=0 }
                END; { Else of VisiCour=VisiPrec }
            VisiPrec := VisiCour;
            XPrec := XCour;
            YPrec := YCour;
          END; { For Point }
        AretesDeFermeture (XCour,YCour,Xg,Yg)
      END { For Ligne }
  END;

{ Phan trinh bay cac tham so ve hinh ve }
PROCEDURE Affichage;
VAR Aux, Ch : STRING;
  BEGIN
    SetColor($F);
    { $F la dang viet Hexal o he 10 la so 15 ( Co mau trang ) }
    OutTextXY(95,472,
    'X=[-8.2,8.2] Y=[-8.2,8.2] Theta=45 Phi=20 Lig=57 Pts=83')
  END;

BEGIN  { Bat da thu tuc Graph_Demo11}
  EntreeDesParametresDeVision;    { Nhap so lieu can thiet }
  InitialisationsDiverses;        { Khoi tao cac tham so cho chuong trinh }
  InitialiseProjection;           { Khoi tao cac phep chieu }
  RechercheFenetre;               { Tim cua so hien thi thong tin }
  CalculeEchelles;                { Tinh cac ty le }
  EnterGraph(Mode16); { Khoi dong che do do hoa }
  DessineFonction;                { Ve ham }
  Affichage;                      { Trinh bay cac tham bien }
  Attente;                        { Cho bam mot phim }
  CloseGraph                      { Dong che do do hoa }

END; { Ket thuc Graph_Demo11 }

{----------------------------------------------------------------------}
{        CAC MODULE HUONG DAN SU DUNG VA GIOI THIEU CHUONG TRINH       }
{----------------------------------------------------------------------}

{ Thu tuc tao ra man hinh gioi thieu ve cach
  su dung va ban quyen cua chuong trinh }
PROCEDURE Introduce_4;
  BEGIN
    SetTextColor(Yellow,Blue);
    SetBorderColor(Yellow);
    ClrScr;
    Frame(2,1,79,25,1);
    SayXY(30,1,White,Red,'Graphics Demontration');
    SetTextColor(White,Blue);
    WriteXY(14,2,'Sau ¦¡y t£i xin giíi thiÖu víi cªc b«n cªc  ch¥¤ng  tr×nh');
    WriteXY(10,3,'¦å häa DEMO ¦¥îc viÕt b¬ng Borland Pascal Ver 7.0  trong  chÕ');
    WriteXY(10,4,'¦é ¦å häa Text (TEXT MODE) v§ chÕ ¦é ¦å häa GRAPHICS.');
    SayXY(13,6,LightGreen,Blue,'* ChÕ ¦é TEXT :');
    SetTextColor(White,Blue);
    WriteXY(14,7,'Ch¥¤ng tr×nh thÓ hiÖn m§u s¯c r½t phong phó cña chÕ ¦é ¦å');
    WriteXY(10,8,'häa trong chÕ ¦é TEXT . Ch¥¤ng tr×nh sö dông cªc ký tù  trong');
    WriteXY(10,9,'b¨ng m© ASCII ¦Ó trang trÝ m§n h×nh v§ viÕt ch÷ to trong Text');
    SayXY(13,10,LightGreen,Blue,'* ChÕ ¦é GRAPHICS_2D :');
    SetTextColor(White,Blue);
    WriteXY(14,11,'Ch¥¤ng tr×nh sÏ giíi thiÖu cªch vÏ h×nh trong kh£ng  gian'); ;
    WriteXY(10,12,'2  chiÒu ch¥¤ng tr×nh sö dông cªc thu¾t toªn ¦Ó thiÕt kÕ  cªc');
    WriteXY(10,13,'h×nh . Cªc ch¥¤ng tr×nh trong chÕ ¦é ¦å häa hai chiÒu  chØ ¦Ò');
    WriteXY(10,14,'c¾p ¦Õn cªc h×nh ¨nh trong hÖ täa ¦é x, y v§ cªc ch¥¤ng tr×nh');
    WriteXY(10,15,'vÏ h×nh trong mµt ph­ng.');
    SayXY(13,16,LightGreen,Blue,'* ChÕ ¦é GRAPHICS_3D :');
    SetTextColor(White,Blue);
    WriteXY(14,17,'Ch¥¤ng tr×nh sÏ giíi thiÖu cªch vÏ h×nh trong kh£ng  gian');
    WriteXY(10,18,'3 chiÒu. V× lý do h«n chÕ vÒ thêi gian n¢n t£i chØ giíi thiÖu');
    WriteXY(10,19,'víi cªc b«n mét ch¥¤ng tr×nh vÏ h×nh trong kh£ng gian 3 chiÒu');
    WriteXY(10,20,'n§y th£i , cªc ch¥¤ng tr×nh ¦å häa cßn l«i sÏ ¦¥îc  tr×nh b§y');
    WriteXY(10,21,'trong ph¶n ¦å häa cña ph¶n mÒm Borland C  3.2 v× ph¶n mÒm n§y');
    WriteXY(10,22,'r½t h÷u Ých cho viÖc thiÕt kÕ cªc h×nh trong chÕ ¦é ¦å  häa 3');
    WriteXY(10,23,'chiÒu n¢n cªc b«n th£ng c¨m. Ch¥¤ng tr×nh ë ph¶n n§y l§ m£ t¨');
    WriteXY(10,24,'h×nh ¨nh cña mét c¡y n½m bªo b©o m§ cªc b«n th¥êng hay gµp.');
    SayXY(27,25,White,Red,'Press <ENTER> To Continue');
    GotoXY(1,1); { De cho man hinh giu nguyen vi tri cu }
    Readln
  END;

{ Thu tuc tao ra loi huong dan su dung trong
  cac chuong trinh GRAPHICS_DEMO }
PROCEDURE Introduce_5;
VAR Key : Char;
  BEGIN
    BoxShadow(18,8,61,16,Yellow,Cyan,LightGray,Black,2);
    SayXY(34,8,White,Red,'H¥íng D¸n');
    SetTextColor(Black,Cyan);
    WriteXY(20,10,'Khi cªc hép tho«i ch«y ch¥¤ng tr×nh hiÖn');
    WriteXY(20,11,'ra th× b«n chØ c¶n l§m theo cªc chØ  d¸n');
    WriteXY(20,12,'tr¢n m§n h×nh . NÕu kh£ng cã cªc chØ d¸n');
    WriteXY(20,13,'th× b«n ¦õng quª lo l¯ng , h©y b×nh tÜnh');
    WriteXY(20,14,'gâ phÝm <ENTER> l§ phÇ re !.');
    Readln;
    SetBorderColor(Black)
  END;

{ Thu tuc cho thi hanh cac chuong trinh GRAPHICS DEMONTRATION }
PROCEDURE Graphics;
VAR Key : Char;
  BEGIN
    CurSorOff;
    Introduce_4;
    Introduce_5;
    SetTextColor(Black,Blue);
    ClrScr;
    BoxShade(12,10,67,15,Cyan,Yellow,1);
    SayXY(34,10,White,Red,'Graph Demo1');
    SetTextColor(Black,Cyan);
    WriteXY(14,11,'Ch¥¤ng tr×nh Graph_Demo1,m£ t¨ thñ thu¾t viÕt ch÷ to');
    WriteXY(14,12,'trong chÕ ¦é v n b¨n. Ch¥¤ng tr×nh thÓ hiÖn kh¨ n ng');
    WriteXY(14,13,'¦µc biÖt cña ¦å häa ë chÕ ¦é TEXT ¦ã l§ viÖc t«o  ra');
    WriteXY(14,14,'cªc dßng ch÷ m§u th¾t to duy chuyÓn tr¢n m§n h×nh.');
    SayXY(20,18,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo0;
    Graph_Demo1;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(12,10,69,15,Cyan,Yellow,1);
    SayXY(34,10,White,Red,'Graph Demo2');
    SetTextColor(Black,Cyan);
    WriteXY(14,11,'Ch¥¤ng tr×nh Graph_Demo2 l§ m£ t¨ trªi ¦½t chuyÓn ¦éng');
    WriteXY(14,12,'xung quanh mµt trêi. Tr¢n b¶u trêi cã cªc ng£i sao lóc');
    WriteXY(14,13,'·n lóc hiÖn tr£ng r½t ¦Ñp . B«n h©y l¥u ý ¦Õn cªc ng£i');
    WriteXY(14,14,'sao lín v§ hiÖu øng cña ch¥¤ng tr×nh khi nã hiÖn l¢n.');
    SayXY(20,18,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo2;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,70,15,Cyan,Yellow,1);
    SayXY(34,10,White,Red,'Graph Demo3');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo3  l§ m£ t¨ mét chïm phªo b£ng lín');
    WriteXY(12,12,'¦ang ho«t ¦éng ë gi÷a m§n h×nh. B«n h©y chó ý v§o t¡m cña');
    WriteXY(12,13,'cªc chïm phªo b£ng n§y b«n sÏ tr£ng th½y nh÷ng  h×nh  ¨nh');
    WriteXY(12,14,'ng¸u nhi¢n r½t ¦Ñp. Ngo§i ra cßn cã 4 chïm phªo b£ng nhá.');
    SayXY(19,18,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo3;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,70,15,Cyan,Yellow,1);
    SayXY(34,10,White,Red,'Graph Demo4');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo4 , m£ t¨ cªch sö dông hai thñ tôc');
    WriteXY(12,12,'MoveTo v§ LineRel . Ch¥¤ng tr×nh vÏ cªc h×nh ch÷ nh¾t dån');
    WriteXY(12,13,'v§o t¡m cña m§n h×nh víi m§u thay ¦æi ng¸u nhi¢n, ë chÝnh');
    WriteXY(12,14,'gi÷a m§n h×nh l§ mét h×nh thoi cè ¦Þnh cã m§u biÕn ¦æi.');
    SayXY(20,18,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo4;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,67,15,Cyan,Yellow,1);
    SayXY(32,10,White,Red,'Graph Demo5');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo5, m£ t¨ cªch vÏ ¦¥êng KOCH_GON');
    WriteXY(12,12,'l¾p th§nh mét tam giªc ¦Òu cã m§u thay ¦æi ng¸u nhi¢n.');
    WriteXY(12,13,'ŸÓ vÏ ¦¥îc h×nh n§y b«n c¶n thiÕt kÕ s®n mét v§i  kiÓu');
    WriteXY(12,14,'chu·n v§ mét sè thñ tôc tiÖn lîi trong hÖ täa ¦é thùc.');
    SayXY(19,18,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo5;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,69,15,Cyan,Yellow,1);
    SayXY(34,10,White,Red,'Graph Demo6');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo6 , m£ pháng mét ¦Üa bay ¦ang bay');
    WriteXY(12,12,'trong b¶u trêi vò trô ¦¶y sao. Ÿ¡y l§ ph¥¤ng phªp c¯t v§');
    WriteXY(12,13,'dªn ¦Ó t«o h×nh ¨nh chuyÓn ¦éng,  mét kü thu¾t r½t tuyÖt');
    WriteXY(12,14,'h¨o trong ¦å häa. Dïng thñ tôc GetImage,GetMem,PutImage.');
    SayXY(19,18,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo6;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,72,15,Cyan,Yellow,1);
    SayXY(35,10,White,Red,'Graph Demo7');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo7 , sö dông cªc thñ tôc BAR3D v§ cªc ');
    WriteXY(12,12,'thñ tôc Palette ¦Ó trang trÝ m§n h×nh . Ch¥¤ng tr×nh vÏ cªc');
    WriteXY(12,13,'BAR cã m§u cè ¦Þnh, sau ¦ã cho tham sè Color ¦¥îc thiÕt l¾p');
    WriteXY(12,14,'tr¥íc ¦ã thay ¦æi mét cªch ng¸u nhi¢n cho ta tr£ng th½y ¦Ñp.');
    SayXY(20,18,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo7;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,72,17,Cyan,Yellow,1);
    SayXY(35,10,White,Red,'Graph Demo8');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo8 ,  ch¥¤ng tr×nh m£ t¨ viÖc l§m ¨nh');
    WriteXY(12,12,'chuyÓn ¦éng trong chÕ ¦é Graphics m§ kh£ng dïng cªc thñ tôc');
    WriteXY(12,13,'GetImage, GetMem,PutImage nh¥ th¥êng lÖ. Th¾t ra th× ch¥¤ng');
    WriteXY(12,14,'tr×nh còng kh£ng cã g× gh¢ gím l¯m, ¦ã l§ viÖc vÏ cªc ¦¥êng');
    WriteXY(12,15,'th­ng sau ¦ã dïng thu¾t toªn che v§ c¾p nh¾t  th¥êng  xuy¢n');
    WriteXY(12,16,'cªc ¦o«n th­ng n§y sau mçi l¶n cã sù thay ¦æi vÞ trÝ.');
    SayXY(20,20,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo8;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,74,17,Cyan,Yellow,1);
    SayXY(36,10,White,Red,'Graph Demo9');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo9 , ch¥¤ng tr×nh b§n víi cªc b«n vÒ kü');
    WriteXY(12,12,'thu¾t ¦å häa trong kh£ng gian 2 chiÒu. Ch¥¤ng tr×nh m£ t¨ mét');
    WriteXY(12,13,'h×nh Bolygone, gi¨i thu¾t cña ch¥¤ng tr×nh n§y l§ sö dông cªc ');
    WriteXY(12,14,'thu¾t toªn cho mét cung cña h×nh trßn sau ¦ã vÏ cªc cung khªc');
    WriteXY(12,15,'néi tiÕp theo cªc cung tr¥íc ¦ã .  Sè cung vÏ ¦¥îc gäi l§ b¾c');
    WriteXY(12,16,'cña Bolygone. Kho¨ng cªch gi÷a cªc cung ¦¥îc gäi l§ b¥íc t ng.');
    SayXY(20,20,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo9;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,73,17,Cyan,Yellow,1);
    SayXY(35,10,White,Red,'Graph Demo10');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo10 ,  ch¥¤ng tr×nh b§n víi cªc b«n vÒ');
    WriteXY(12,12,'kü thu¾t ¦å häa trong kh£ng gian 2 chiÒu. Ch¥¤ng tr×nh m£ t¨');
    WriteXY(12,13,'mét h×nh cu¨ KÝnh V«n Hoa, gi¨i thu¾t cña ch¥¤ng tr×nh l§ sö');
    WriteXY(12,14,'dông cªc ¦a giªc quay xung quanh mét ¦iÓm cè ¦Þnh  theo  cªc');
    WriteXY(12,15,'quy t¯c sau : cªc tam giªc ch®n sÏ quay cïng  chiÒu kim ¦ång');
    WriteXY(12,16,'hå v§ t½t c¨ cªc tam giªc lÇ sÏ quay ng¥îc chiÒu kim ¦ång hå.');
    SayXY(20,20,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo10;
    Clear(1,1,80,25,Blue);
    CurSorOff;
    BoxShade(10,10,72,17,Cyan,Yellow,1);
    SayXY(35,10,White,Red,'Graph Demo11');
    SetTextColor(Black,Cyan);
    WriteXY(12,11,'Ch¥¤ng tr×nh Graph_Demo11 , ch¥¤ng tr×nh b§n víi cªc b«n vÒ');
    WriteXY(12,12,'kü thu¾t ¦å häa trong kh£ng gian 3 chiÒu.Ch¥¤ng tr×nh m£ t¨');
    WriteXY(12,13,'mét h×nh ¨nh cña c¡y n½m bªo b©o m§ cªc b«n th¥êng th½y sau');
    WriteXY(12,14,'c¤n m¥a.Thu¾t gi¨i cña ch¥¤ng tr×nh n§y khª phøc t«p vÒ mµt');
    WriteXY(12,15,'toªn häc,¦ã l§ viÖc sö dông cªc phÐp chiÕu trong kh£ng gian');
    WriteXY(12,16,'3 chiÒu. C¶n ph¨i thiÕt kÕ s®n cªc thñ tôc vÒ phÐp chiÕu.');
    SayXY(20,20,Yellow,Blue,'Press Any Key To Continue Or <ESC> To Quits');
    Key:=ReadKey;
    IF Key=#27 THEN Main_Menu;
    Graph_Demo11
  END;

{ Thu tuc gioi thieu huong dan su dung }
PROCEDURE About;
VAR
  I   : Integer;
  Key : Char;
  BEGIN
    RestoreCrtMode; { Phuc hoi lai che do van ban }
    SetTextColor(Yellow,Blue);
    SetBorderColor(Yellow);
    ClrScr;
    CurSorOff;
    Frame(2,1,79,25,1);
    SetTextColor(LightCyan,Blue);
    WriteXY(18,2,'Cªm ¤n b«n ¦© chän môc n§y cña ch¥¤ng tr×nh');
    SetTextColor(Yellow,Blue);
    WriteXY(5,3,'- Ch¥¤ng tr×nh gåm cã 6 ph¶n chÝnh :');
    WriteXY(5,4,'* Ph¶n I   : Gi¨i Ph¥¤ng Tr×nh B¾c Nh½t cã d«ng');
    WriteXY(18,5,'A*x+B=0. Víi cªc hÖ sè A,B l§ sè nguy¢n ¦¥îc nh¾p tõ b§n phÝm');
    WriteXY(5,6,'* Ph¶n II  : Gi¨i Ph¥¤ng Tr×nh B¾c Hai cã d«ng');
    WriteXY(18,7,'A*x^2+B*x+C=0. Víi cªc hÖ sè A,B v§ C l§ cªc sè nguy¢n ¦¥îc');
    WriteXY(18,8,'nh¾p tõ b§n phÝm.');
    WriteXY(5,9,'* Ph¶n III : Gi¨i HÖ Ph¥¤ng TrÝnh TuyÕn TÝnh cã d«ng');
    WriteXY(18,10,'A1*x+B1*y+C1=0');
    WriteXY(18,11,'A2*x+B2*y+C2=0.');
    WriteXY(18,12,'Víi cªc hÖ sè A1,A2,B1,B2,C1 v§ C1 l§ cªc sè nguy¢n ¦¥îc nh¾p');
    WriteXY(18,13,'tõ b§n phÝm. (A1,A2,B1,B2,C1 v§ C2 kh£ng ¦ång thêi b¬ng 0)');
    WriteXY(5,14,'* Ph¶n IV  : TÝnh Giai Thõa cña N! (N<=13) víi N l§ sè nguy¢n d¥¤ng ¦¥îc ');
    WriteXY(18,15,'nh¾p tõ b§n phÝm.');
    WriteXY(5,16,'* Ph¶n V   : TÝnh X Mò Y Víi X (X>0) v§ Y ¦¥îc nh¾p tõ b§n phÝm.');
    WriteXY(5,17,'* Ph¶n VI  : Gåm cã cªc ch¥¤ng tr×nh ¦å häa Demontration.');
    WriteXY(28,20,'Khoa C£ng NghÖ Th£ng Tin');
    WriteXY(23,21,'Tr¥êng Ÿ«i Häc Kü Thu¾t C£ng NghÖ');
    WriteXY(20,22,'(C) Copyright by Student NguyÔn Ngäc V¡n');
    WriteXY(35,23,'Líp 00ŸTH01');
    WriteXY(29,19,'Ÿ¡y l§ phi¢n b¨n ch«y thö nghiÖm');
    WriteXY(2,18,#195);
    WriteXY(78,18,#196#180);
    WriteXY(3,18,Replicate(#196,75));
    SayXY(19,19,LightGreen+Blink,Blue,'WARNING!');
    SayXY(30,25,White,Red,'Press <ENTER> to Exit');
    SayXY(31,1,White,Red,'H¥íng D¸n Sö Dông');
    RunChar1;
    Readln;
    FOR i:=12 DOWNTO 0 DO
      BEGIN
        Clear(3*i,i,80-3*i,26-i,Random(i) MOD 7+1);
        Sound(212*i);
        Delay(120-i);
        Nosound
      END;
    SetTextColor(Yellow,Blue);
    ClrScr;
    Frame(2,1,79,25,1);
    WriteXY(20,6,'Tr¥êng Ÿ«i Häc Kü Thu¾t C£ng NghÖ');
    WriteXY(25,7,'C¤ së I  : 110 Cao Th¯ng Qu¾n 3');
    WriteXY(25,8,'C¤ së II : 144/24 ŸiÖn Bi¢n Phñ');
    WriteXY(36,9,'Qu¾n B×nh Th«nh');
    WriteXY(20,10,'Mäi chi tiÕt xin li¢n hÖ t«i ¦Þa chØ');
    WriteXY(25,11,'- Phone   : 9300061');
    WriteXY(25,12,'- Website : http://www.unitech.vnn.vn');
    WriteXY(25,13,'- Email   : nguyenvan@dth01.vnn.vn');
    WriteXY(20,14,'Hoµc li¢n hÖ trùc tiÕp víi tªc gi¨');
    WriteXY(20,15,'t«i c¤ së II phßng 403, l¶u 4.');
    WriteXY(2,18,#195);
    WriteXY(78,18,#196#180);
    WriteXY(3,18,Replicate(#196,75));
    WriteXY(7,19,'Xin l¥u ý : Ch¥¤ng tr×nh l§ phi¢n b¨n ch«y thö nghiÖm. C½m sao chÐp');
    WriteXY(7,20,'mét ph¶n hay c¨ ch¥¤ng tr×nh d¥íi b½t kú h×nh thøc n§o. Mäi vi ph«m');
    WriteXY(7,21,'sÏ bÞ truy tè tr¥íc héi ¦ång cña nh§ tr¥êng. NÕu cªc b«n cã nhu c¶u');
    WriteXY(7,22,'vÒ ch¥¤ng tr×nh nguån xin vui lßng li¢n hÖ t«i cªc ¦Þa chØ tr¢n.');
    SayXY(30,25,White,Red,'Press <ENTER> to Exit');
    SayXY(36,1,White,Red,'B¨n QuyÒn');
    RunChar2;
    Readln;
    NextPage;
    SetBorderColor(Black);
    Main_Menu
  END;

{ Thuc tuc thay loi ket chuong trinh }
PROCEDURE SayGoodBye;
VAR
  OldCursor : Word;
  i         : Byte;
  BEGIN
    OldCursor:=GetCursor;
    CurSorOff;
    SetBorderColor(LightCyan);
    FillFrame(1,1,80,25,White,#206);
    BoxShadow(15,8,68,16,Yellow,Cyan,DarkGray,Black,2);
    SayXY(37,8,White,Red,'GoodBye');
    SayXY(27,16,White,Red,'Press <ENTER> To Exit Program');
    SetTextColor(Black,Cyan);
    WriteXY(17,10,'R½t cªm ¤n cªc b«n ¦© ñng hé, sö dông ch¥¤ng tr×nh');
    WriteXY(17,11,'cña t£i. Mong cªc b«n gãp ý vÒ néi dung cña ch¥¤ng');
    WriteXY(17,12,'tr×nh ¦Ó ch¥¤ng tr×nh ¦¥îc ho§n thiÖn h¤n. HÑn gµp');
    WriteXY(17,13,'l«i cªc b«n ë ph¶n l¾p tr×nh chuy¢n s¡u ( Ph¶n l¾p');
    WriteXY(17,14,'tr×nh hÖ thèng C/C++ v§ Assembler ).');
    Readln;
    FOR i:=12 DOWNTO 0 DO
      BEGIN
        Clear(3*i,i,80-3*i,26-i,Blue);
        Delay(100)
      END;
    SetBorderColor(Black);
    TextMode(LastMode);
    Halt; { Dung ngay chuong trinh dang hoat dong va tro ve He Dieu Hanh }
    SetCursor(OldCursor)
  END;

{ Tao bang Menu chinh cua chuong trinh }
PROCEDURE Main_Menu;
VAR CenterY : Word;
    N,Chon  : Integer;
    Ch      : Char;

  BEGIN
    REPEAT
      EnterGraph(Mode256);
      CenterX:=GetmaxX DIV 2;
      CenterY:=GetmaxY DIV 2;
      SetFillStyle(SolidFill,LightGray);
      Bar(0,0,GetMaxX,GetMaxY);
      ButtonDown(100,40,600,410,LightBlue,Green,'');
      SetTextStyle(DefaultFont,HorizDir,1);
      SetTextJustify(CenterText,CenterText);
      SetFillStyle(1,LightCyan);
      Rectangle(98,38,598,412);
      { Thiet lap bieu tuong cua truong }
      SetFillStyle(SolidFill,Yellow);
      SetColor(Yellow);
      Circle(300,165,70);
      FloodFill(320,165,Yellow);
      SetFillStyle(SolidFill,LightRed);
      SetColor(LightRed);
      Circle(300,165,10);
      FloodFill(300,165,LightRed);
      EllipseBK(300,165,20,70,0,LightRed);
      EllipseBK(300,165,20,70,60,LightRed);
      EllipseBK(300,165,20,70,120,LightRed);
      { Vien cua nut lom }
      ButtonUp(130,60,195,80,LightCyan,Red,'NTBN');
      ButtonUp(130,110,195,130,LightGray,White,'PTBH');
      ButtonUp(130,160,195,180,LightGray,White,'HEPT');
      ButtonUp(130,210,195,230,LightGray,White,'TINHN!');
      ButtonUp(130,260,195,280,LightGray,White,'XMUY');
      ButtonUp(130,310,195,330,LightGray,White,'GRAPHIC');
      ButtonUp(130,360,195,380,LightGray,White,'QUIT');
      SetFillStyle(SolidFill,White);
      Bar(0,0,GetMaxX,20);
      {Thanh trang tren cung }
      Bar(0,GetmaxY-20,GetMaxX,GetMaxY);
      {Thanh trang duoi cung}
      SetTextStyle(DefaultFont,HorizDir,1);
      SetTextJustify(CenterText,CenterText);
      SetColor(Magenta);
      OuttextXY(GetMaxX DIV 2,12,
      'CHUONG TRINH KIEM TRA CAC THUAT TOAN DON GIAN');
      OutTextXY(GetMaxX DIV 2,GetMaxY-8,
      'Copyright (c) By Student NGUYEN NGOC VAN All Rights Reserved');
      SetFillStyle(SolidFill,Yellow);
      Bar(100,425,600,445);
      SetColor(LightRed);
      Rectangle(100,425,600,445);
      SetColor(Blue);
      OutTextXY(GetMaxX DIV 4,GetMaxY-43,'WARNING! :');
      SetColor(LightRed);
      OutTextXY((GetMaxX DIV 3)*2-35,GetMaxY-43,
      'Chuong trinh nay la phien ban chay thu nghiem');
      SetColor(White);
      Rectangle(225,70,570,260);  {HCN lon ben tren}
      Rectangle(225,310,570,380); {HCN nho ben duoi}
      SetColor(Yellow);
      OutTextXY(400,325,'HELP ?');
      SetColor(White);
      SetTextJustify(LeftText,CenterText);
      SetColor(LightGreen);
      OutTextXY(455,345,'Q');
      OutTextXY(455,365,'F1');
      OutTextXY(240,345,#205#205#16);
      OutTextXY(240,365,'Enter');
      SetColor(White);
      OutTextXY(240,345,'      : Seclect Item         : Quit');
      OutTextXY(240,365,'      : Process              : About');
      WriteShade(468,100,'TRI THUC',DefaultFont,3,LightRed,White);
      WriteShade(468,165,'DAO DUC',DefaultFont,3,LightRed,White);
      WriteShade(468,230,'SANG TAO',DefaultFont,3,LightRed,White);
      WriteShade(45,85,'K',DefaultFont,8,LightRed,Yellow);
      WriteShade(45,175,'T',DefaultFont,8,LightRed,Yellow);
      WriteShade(45,265,'C',DefaultFont,8,LightRed,Yellow);
      WriteShade(45,365,'N',DefaultFont,8,LightRed,Yellow);
      N:=1;
      REPEAT
         Ch:=Readkey;
         IF Ch=#0 THEN Ch:=Readkey;
         { De phong bam vao cac phim chuc nang }
         CASE Ch OF
           #72: IF N=1 THEN Inc(N,6) ELSE Dec(N); { Ma phim Up }
           #80: IF N=7 THEN N:=1     ELSE Inc(N); { Ma phim Down }
           #71: N:=1; { Ma phim Home }
           #79: N:=7; { Ma phim End }
           #59: About
         END;
         CASE N OF
            1: BEGIN { Phuong trinh bat nhat }
                 ButtonUp(130,60,195,80,LightCyan,Red,'NTBN');
                 ButtonUp(130,110,195,130,LightGray,White,'PTBH');
                 ButtonUp(130,160,195,180,LightGray,White,'HEPT');
		 ButtonUp(130,210,195,230,LightGray,White,'TINHN!');
                 ButtonUp(130,260,195,280,LightGray,White,'XMUY');
                 ButtonUp(130,310,195,330,LightGray,White,'GRAPHIC');
                 ButtonUp(130,360,195,380,LightGray,White,'QUIT');
                 Chon:=1
               END;

            2: BEGIN { Phuong trinh Bat hai }
                 ButtonUp(130,60,195,80,LightGray,White,'NTBN');
                 ButtonUp(130,110,195,130,LightCyan,Red,'PTBH');
                 ButtonUp(130,160,195,180,LightGray,White,'HEPT');
		 ButtonUp(130,210,195,230,LightGray,White,'TINHN!');
                 ButtonUp(130,260,195,280,LightGray,White,'XMUY');
                 ButtonUp(130,310,195,330,LightGray,White,'GRAPHIC');
                 ButtonUp(130,360,195,380,LightGray,White,'QUIT');
                 Chon:=2
               END;

            3: BEGIN { He phuong trinh }
                 ButtonUp(130,60,195,80,LightGray,White,'NTBN');
                 ButtonUp(130,110,195,130,LightGray,White,'PTBH');
                 ButtonUp(130,160,195,180,LightCyan,Red,'HEPT');
		 ButtonUp(130,210,195,230,LightGray,White,'TINHN!');
                 ButtonUp(130,260,195,280,LightGray,White,'XMUY');
                 ButtonUp(130,310,195,330,LightGray,White,'GRAPHIC');
                 ButtonUp(130,360,195,380,LightGray,White,'QUIT');
                 Chon:=3
               END;

            4: BEGIN { Tinh giai thua }
                 ButtonUp(130,60,195,80,LightGray,White,'NTBN');
                 ButtonUp(130,110,195,130,LightGray,White,'PTBH');
                 ButtonUp(130,160,195,180,LightGray,White,'HEPT');
                 ButtonUp(130,210,195,230,LightCyan,Red,'TINHN!');
                 ButtonUp(130,260,195,280,LightGray,White,'XMUY');
                 ButtonUp(130,310,195,330,LightGray,White,'GRAPHIC');
                 ButtonUp(130,360,195,380,LightGray,White,'QUIT');
                 Chon:=4
               END;

            5: BEGIN { Tinh X mu Y }
                 ButtonUp(130,60,195,80,LightGray,White,'NTBN');
                 ButtonUp(130,110,195,130,LightGray,White,'PTBH');
                 ButtonUp(130,160,195,180,LightGray,White,'HEPT');
                 ButtonUp(130,210,195,230,LightGray,White,'TINHN!');
                 ButtonUp(130,260,195,280,LightCyan,Red,'XMUY');
                 ButtonUp(130,310,195,330,LightGray,White,'GRAPHIC');
                 ButtonUp(130,360,195,380,LightGray,White,'QUIT');
                 Chon:=5
               END;

            6: BEGIN { Cac chuong trinh do hoa DEMO }
                 ButtonUp(130,60,195,80,LightGray,White,'NTBN');
                 ButtonUp(130,110,195,130,LightGray,White,'PTBH');
                 ButtonUp(130,160,195,180,LightGray,White,'HEPT');
		 ButtonUp(130,210,195,230,LightGray,White,'TINHN!');
                 ButtonUp(130,260,195,280,LightGray,White,'XMUY');
                 ButtonUp(130,310,195,330,LightCyan,Red,'GRAPHIC');
                 ButtonUp(130,360,195,380,LightGray,White,'QUIT');
                 Chon:=6
               END;

            7: BEGIN { Thoat chuong trinh }
                 ButtonUp(130,60,195,80,LightGray,White,'NTBN');
                 ButtonUp(130,110,195,130,LightGray,White,'PTBH');
                 ButtonUp(130,160,195,180,LightGray,White,'HEPT');
                 ButtonUp(130,210,195,230,LightGray,White,'TINHN!');
                 ButtonUp(130,260,195,280,LightGray,White,'XMUY');
                 ButtonUp(130,310,195,330,LightGray,White,'GRAPHIC');
                 ButtonUp(130,360,195,380,LightCyan,Red,'QUIT');
                 Chon:=7
               END;
         END;
	 { Cac phim tat, rieng 3 phim o dau menu la thuan
      	   tien nen khong cai Driver do do khong su dung
	   duoc mac du no co mau do. }
         CASE UpCase(Ch) OF
           'T' : BEGIN Chon := 4; Ch := #13 END;
           'X' : BEGIN Chon := 5; Ch := #13 END;
           'G' : BEGIN Chon := 6; Ch := #13 END;
           'Q' : BEGIN Chon := 7; Ch := #13 END
         END
      UNTIL Ch=#13;
      CloseGraph;
        CASE Chon OF
           1: Module_1;  { Giai phuong trinh bac nhat }
           2: Module_2;  { Giai phuong trinh bac hai }
           3: Module_3;  { Giai he phuong trinh tuyen tinh }
           4: Module_4;  { Tinh giai thua }
           5: Module_5;  { Tinh X Mu Y }
           6: Graphics;  { Huong dan su dung va cac chuong trinh Graph_Demo }
           7: SayGoodBye { Ket thuc chuong trinh va quay ve He Dieu Hanh    }
        END
    UNTIL Chon = 7
END;

{ Kiem tra thoi gian su dung chuong trinh }
PROCEDURE CheckPeriod;
VAR CurrDay, CurrMonth,
    CurrYear, DayOfWeek : Word;
    Fp                  : FILE OF ShortInt;
    OldMonth, OldDay    : ShortInt;
  BEGIN
     GetDate(CurrYear, CurrMonth, CurrDay, DayOfWeek);
     {$I-}
     Assign(Fp,'C:\WINDOWS\register.dat');
     Reset(Fp);
     {$I+}
     IF IOResult <> 0 THEN
        BEGIN
           SetBorderColor(55);
           SetTextColor(Blue,White);
           ClrScr;
           SayXY(30, 10, $0F, $01, 'SYSTEM ERROR!');
           SayXY(20, 12, $0C, $01, 'Cannot reading from file register.dat');
           SayXY(20, 13, $0C, $01, 'Please press ENTER key to exit program');
           Readln;
           EndProgram;
        END;
     Read(Fp, OldDay, OldMonth);
     Close(Fp);
     IF CurrMonth - OldMonth = 0 THEN
        BEGIN
           IF CurrDay - OldDay >= 7  THEN
              BEGIN
                SetTextColor($0F,$01);
                SetBorderColor(47);
                SetCursor($2020);
                Clrscr;
                Sound(2000);
                Delay(200);
                Nosound;
                SayXY(32,10,$0F,$01,'SYSTEM BUSY!');
                SayXY(10,12,$0E,$01,'This program does not correct version. If you continue using it');
                SayXY(10,13,$0E,$01,'please register with the author. Hit ENTER key to exit program!');
                Readln;
                EndProgram;
              END
        END
     ELSE
        BEGIN
           IF (CurrDay + 31)  - OldDay >= 7  THEN
              BEGIN
                SetTextColor($0F,$01);
                SetBorderColor(47);
                SetCursor($2020);
                Clrscr;
                Sound(2000);
                Delay(200);
                Nosound;
                SayXY(32,10,$0F,$01,'SYSTEM BUSY!');
                SayXY(10,12,$0E,$01,'This program does not correct version. If you continue using it');
                SayXY(10,13,$0E,$01,'please register with the author. Hit ENTER key to exit program!');
                Readln;
                EndProgram;
              END
        END
  END;

{ Hien thong bao dang ky ban quyen }
PROCEDURE Register;
   BEGIN
     SetBorderColor(58);
     SetTextColor($0F,$01);
     SetCursor($2020);
     ClrScr;
     FillFrame(1,1,80,25,$1A,#176);
     BoxShadow(3,10,77,16,$0F,$04,$07,$00,2);
     SayXY(32,10,$0E,$01,'Unregister');
     SayXY(9,12,$0F,$04,'You have to register this copy in order to continue using it');
     SayXY(14,13,$0A,$04,'(The copy registered does not showing this message)');
     SayXY(5,14,$0F,$04,'You can register this program at any programs into the topics program');
     Readln;
   END;

BEGIN  { Chuong trinh chinh AlGORITHM.PAS }
  Register;       { Hien thong bao nhac nho dang ky }
  CheckPeriod;    { Kiem tra thoi han su dung }
  Enter_Password; { Kiem tra mat khau }
  Start_Program;  { Bat dau chuong trinh }
  Introduce_1;    { Man hinh gioi thieu 1 }
  Introduce_2;    { Man hinh gioi thieu 2 }
  Main_Menu;      { Menu chinh chuong trinh }
END.
