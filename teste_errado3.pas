program erro03;
var
   x: integer;
begin
   x := 10;
   if x > 0 then
      x := 0
   { <-- Faltou o 'end' que fecha o bloco principal }
.