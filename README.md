MD Router Control JS

Opens your router's web interface by one click on the tray icon.\
Password for router is stored by libsecret.\
After closing the interface window, checks connection to the test address and changes tray icon.\
Based on webkit2gtk. This version for JavaScript authorization on router. TP-Link by default.\
\
After closing the program, a configuration file will appear. ~/.config/mdrctrl.conf\
It will contain settings for scripts.\
\
authScript - Authorization script that contains username and password.\
The script must contain placeholders for substitution. \[USERNAME\] and \[PASSWORD\]\
It can be a password only.\
For example, $\("\#pc-login-password"\)\.val\("\[PASSWORD\]"\); $\("\#pc-login-btn"\)\.click\(\);\
\
exitScriptSign - Script that should return a string as a sign that you have been authorized and are on a working page.\
For example, $\("\#topLogout"\)\.attr\("href"\);\
\
exitScript - Exit script.\
For example, $\("\#topLogout"\).click\(\);\
\
To find scripts, you need to look at the page source in a browser and know a little JavaScript.\
\
\
MD Router Control JS is provided "as is", with absolutely no warranty expressed or implied. 
Any use is at your own risk. Author not responsible for any damage or lost data.
Permission to use or copy this software for any purpose is hereby granted without fee. 
Permission to modify the code and to distribute modified code is also granted without 
any restrictions.
