# ISP
Intercepting Shell Program

An intercepting shell program supporting composition of two commands.

Includes two modes:\
In first mode, <b>normal mode</b>, a single unnamed Linux pipe will used for communication, and it will be used directly by the child processes. That means, the output of one child is fed directly to the pipe from where the second child will get the data. You will enable this by I/O re-direction.\

In the second mode (<b>tapped mode</b>), two Linux unnamed pipes will be used and data will flow indirectly between child processes. The main process will be on the data flow path. The main process will create two pipes. The output of the first child process will be directed to the first pipe, from where the main process will read the incoming stream of bytes (characters). The main process will write those characters to the second pipe from where the second child will take the input. The main process will read from first pipe and write to second pipe N characters at a time using read and write system calls. N is a value that is given as an argument to the shell program when it is started. N can be between 1 and 4096. 

Hence your shell will have the following parameters:\
<b>isp \<N\> \<mode\></b>\
where <N> is the number of bytes to read/write in one system call and <mode> is the mode of the communication to use. If mode value is 1, then normal communication mode is used. If mode value is 2, tapped communication mode is used.\
An example invocation can be: “./isp 1 2”. That means, we want to read/write 1 byte at a time and tapped mode is used.\

<a href="https://github.com/akaanug/ISP/blob/main/report.pdf" target="_blank">Report</a>
