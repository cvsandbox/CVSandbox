print('Host name: ' .. Host.Name())
print('Host version: ' .. Host.Version())

args = Host.GetArguments( )

for k, v in pairs( args )
do
	print( k,v )
end

function Main()
	print('Hello from Main()')
end
