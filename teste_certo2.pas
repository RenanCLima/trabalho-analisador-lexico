program teste02;
var
   nota, extra: real;
begin
   nota := 7.5;
   extra := 1.0;
   if nota >= 6.0 then
      nota := nota + extra
   else
      nota := 0.0;
end.