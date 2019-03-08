--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which demonstrates communication with device (Arduino)
    connected over serial port.
]]--

local string = require 'string'

serialPort = Host.CreatePluginInstance( 'SerialPort' )
serialPort:SetProperty( 'portName', 'COM8' )

-- Use blocking input, read operations wait up to the configured
-- timeout value
serialPort:SetProperty( 'blockingInput', true )
-- Total Read Timeout = ioTimeoutConstant + ioTimeoutMultiplier * bytesRequested
serialPort:SetProperty( 'ioTimeoutConstant', 50 )
serialPort:SetProperty( 'ioTimeoutMultiplier', 0 )

function Main()

    if serialPort:Connect( ) then
        print( 'Connected' )

        -- Test IsConnected() method
        print( 'IsConnected: ' .. tostring( serialPort:IsConnected( ) ) )

        -- Let Arduino board reset and get ready
        sleep( 1500 )

        -- Switch LED on - send command as string
        sent, status = serialPort:WriteString( 'led_on\n' )

        print( 'status: ' .. tostring( status ) )
        print( 'sent  : ' .. tostring( sent ) )

        strRead, status = serialPort:ReadString( 10 )

        print( 'status  : ' .. tostring( status ) )
        print( 'str read: ' .. strRead )

        -- Switch LED off - sned command as table of bytes
        sent, status = serialPort:Write( { 0x6C, 0x65, 0x64, 0x5F, 0x6F, 0x66, 0x66, 0x0A } )

        print( 'status: ' .. tostring( status ) )
        print( 'sent  : ' .. tostring( sent ) )

        readBuffer, status = serialPort:Read( 10 )

        print( 'status    : ' .. tostring( status ) )
        print( 'bytes read: ' )
        for i=1, #readBuffer do
            print( '[', i, ']=', readBuffer[i] )
        end

        -- Check button state
        sent, status = serialPort:WriteString( 'btn_state\n' )

        print( 'status: ' .. tostring( status ) )
        print( 'sent  : ' .. tostring( sent ) )

        strRead, status = serialPort:ReadString( 10 )

        print( 'status  : ' .. tostring( status ) )
        print( 'str read: ' .. strRead )

        if string.sub( strRead, 1, 1 ) == '1' then
            print( 'button is ON' )
        else
            print( 'button is OFF' )
        end

        -- Test that communication is not blocking
        print( 'Testing timeout' )
        strRead, status = serialPort:ReadString( 10 )

        print( 'status  : ' .. tostring( status ) )
        print( 'str read: ' .. strRead )

        serialPort:Disconnect( )
    end
end
