--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which demonstrates how to talk
    to gamapad device conneted to the system.
]]--

local math = require 'math'

gamepad = Host.CreatePluginInstance( 'Gamepad' )

-- Connected to the first gamapad device
gamepad:SetProperty( 'deviceId', 0 )
if not gamepad:Connect( ) then
    error( "Failed connecting to gamepad" )
end

-- Query name of the device, number of axes and buttons
deviceName   = gamepad:GetProperty( 'deviceName' )
axesCount    = gamepad:GetProperty( 'axesCount' )
buttonsCount = gamepad:GetProperty( 'buttonsCount' )

function Main( )
    -- Query value of all axes and buttons as arrays
    axesValues   = gamepad:GetProperty( 'axesValues' )
    buttonsState = gamepad:GetProperty( 'buttonsState' )

    print( 'X: ' .. tostring( math.floor( axesValues[1] * 100 ) / 100 ) )
    print( 'Y: ' .. tostring( math.floor( axesValues[2] * 100 ) / 100 ) )

    -- Query value of the X axis only
    x = gamepad:GetProperty( 'axesValues', 1 )

    -- Query status of the first button only
    buttonState1 = gamepad:GetProperty( 'buttonsState', 1 )

    if buttonState1 then
        print( 'Button 1 is ON' )
    else
        print( 'Button 1 is OFF' )
    end
end
