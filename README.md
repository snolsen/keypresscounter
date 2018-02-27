# keypresscounter
Small Win API program that keeps track of number of keystrokes and mouseclicks

Keypresscounter counts the total number of keystrokes and mouseclicks as long as it's running.
On execution it will make a minimal window in the top right corner of your screen. It does not show on your task bar or Alt+Tab list.

It can be accessed on the Desktop after minimizing all other windows. 

Keypresscounter will save keystroke statistics to a binary file in the same directory as the .exe with a regular interval.

THE NUMBER OF KEYSTROKES PER KEY AND CLICKS PER MOUSE BUTTON IS THE ONLY INFORMATION THAT IS STORED. 
KEYPRESSCOUNTER IS NOT A KEYLOGGER!

When closed, keypresscounter will write a small plain-text report to a .txt file in the same directory as the .exe.
