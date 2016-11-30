nodejs

Getting error, Error: Cannot find module ‘express’ after npm install
npm link express

http://stackoverflow.com/questions/23006804/getting-error-error-cannot-find-module-express-after-npm-install

Cannot find module 'socket.io' windows 10
npm install --save socket.io
The --save flag will add the given dependency to your application's package.json for easier installation in the future


down vote
accepted
Simple solution (if you are not interested in coming back to the process, just want it to keep running):

nohup node server.js &
Powerful solution (allows you to reconnect to the process if it is interactive):

screen

http://stackoverflow.com/questions/4797050/how-to-run-process-as-background-and-never-die
