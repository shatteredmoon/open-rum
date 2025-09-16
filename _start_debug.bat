rem Runs the debug client and server

@echo off

copy rum_server_debug.* .\export\
copy rum_client_debug.* .\export\

cd export
start rum_server_debug -debug
start rum_client_debug -debug
