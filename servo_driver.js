var angle = process.argv[2] || -1;
var spawn = require('child_process').spawn;
spawn('./bx1_servo/servo', [angle], {
    stdio: 'ignore',
    detached: true
}).unref();
