program Exemplo;
var
   x, y : integer;
   z : real;
begin
   x := 10;
   y := 20;
   z := x + y * 2.5;
   if x > y then
      x := x - 1
   else
      y := y + 1;
   while z <= 100 do
   begin
      z := z * 1.5;
      x := x + 2
   end
end.
