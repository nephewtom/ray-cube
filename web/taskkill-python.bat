FOR /F "tokens=2" %i IN ('tasklist.exe /FI "IMAGENAME eq python.exe" ^| findstr python') DO taskkill.exe /F /PID %i
