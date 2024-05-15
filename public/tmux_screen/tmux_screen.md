## Persistent Terminal Sessions using GNU Screen

[GNU Screen](http://www.gnu.org/software/screen/) is a tool to manage persistent terminal sessions.
It becomes interesting since you will probably end at some moment with the following  scenario:

> you frequently program and run computations on the UL HPC platform _i.e_ on a remote Linux/Unix computer, typically working in six different terminal logins to the access server from your office workstation, cranking up long-running computations that are still not finished and are outputting important information (calculation status or results), when you have 2 interactive jobs running... But it's time to catch the bus and/or the train to go back home.

Probably what you do in the above scenario is to

a. clear and shutdown all running terminal sessions

b. once at home when the kids are in bed, you're logging in again... And have to set up the whole environment again (six logins, 2 interactive jobs etc. )

c. repeat the following morning when you come back to the office.

Enter the long-existing and very simple, but totally indispensable [GNU screen](http://www.gnu.org/software/screen/) command. It has the ability to completely detach running processes from one terminal and reattach it intact (later) from a different terminal login.

### Screen commands

You can start a screen session (_i.e._ creates a single window with a shell in it) with the `screen` command.
Its main command-lines options are listed below:

* `screen`: start a new screen
* `screen -ls`: does not start screen, but prints a list of `pid.tty.host` strings identifying your current screen sessions.
* `screen -r`: resumes a detached screen session
* `screen -x`: attach to a not detached screen session. (Multi display mode _i.e._ when you and another user are trying to access the same session at the same time)


Once within a screen, you can invoke a screen command which consist of a "`CTRL + a`" sequence followed by one other character. The main commands are:

* `CTRL + a c`: (create) creates a new Screen window. The default Screen number is zero.
* `CTRL + a n`: (next) switches to the next window.
* `CTRL + a p`: (prev) switches to the previous window.
* `CTRL + a d`: (detach) detaches from a Screen
* `CTRL + a A`: (title) rename the current window
* `CTRL + a 0-9`: switches between windows 0 through 9.
* `CTRL + a k` or `CTRL + d`: (kill) destroy the current window
* `CTRL + a ?`: (help) display a list of all the command options available for Screen.

## Persistent Terminal Sessions using Tmux

Tmux is a more modern equivalent to GNU screen.

### Tmux commands

You can start a tmux session (_i.e._ creates a single window with a shell in it) with the `tmux` command.
Its main command-lines options are listed below:

* `tmux`: start a new tmux session
* `tmux ls`: does not start tmux, but print the list of the existing sessions.
* `tmux a`: resumes a detached tmux session

Once within a tmux, you can invoke a tmux command which consist of a "`CTRL + b`" sequence followed by one other character. The main commands are:

* `CTRL + b c`: (create) creates a new tmux window. The default tmux number is zero.
* `CTRL + b n`: (next) switches to the next window.
* `CTRL + b p`: (prev) switches to the previous window.
* `CTRL + b d`: (detach) detaches from a session
* `CTRL + b ,`: (title) rename the current window
* `CTRL + b 0-9`: switches between windows 0 through 9.
* `CTRL + d`: (kill) destroy the current window
* `CTRL + b ?`: (help) display a list of all the command options available for tmux.

