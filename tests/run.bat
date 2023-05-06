del .\test-results\res.txt
echo off
for /r %%v in (*.test) do ..\scan "%%v" 2>&1> "test-results/res.txt"
