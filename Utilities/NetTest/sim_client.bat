pushd Debug
REM ----------------------------------------
REM Streaming, no loss

start nettest -client %1 -type 1 -ss 1025 -sp 12

REM ----------------------------------------
REM Streaming, sim packetloss and laggy

start nettest -client %1 -type 1 -ss 4023 -sp 4 -sim 1
start nettest -client %1 -type 1 -ss 120 -sp 10 -sim 2

REM ----------------------------------------
REM test backwards compatability

start nettest -client %1 -type 0 -ss 1025 -sp 12 -sim 2 -linkver 4
start nettest -client %1 -type 0 -ss 1025 -sp 12 -sim 2 -linkver 3

popd