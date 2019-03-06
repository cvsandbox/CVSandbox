--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which demonstrates how to check
    if system is connected to power suply, if battery
    is charging and its current charge level.
]]--

powerInfo = Host.CreatePluginInstance( 'PowerInfo' )
powerInfo:Connect( )

function Main( )
    charge    = powerInfo:GetProperty( 'batteryCharge' )
    connected = powerInfo:GetProperty( 'powerIsConnected' )
    charging  = powerInfo:GetProperty( 'batteryIsCharging' )

    print( 'Batter Charge:   ' .. tostring( charge ) )
    print( 'Power Connected: ' .. tostring( connected ) )
    print( 'Charging:        ' .. tostring( charging ) )
end
