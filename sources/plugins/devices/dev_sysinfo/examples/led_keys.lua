--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which demonstrates how to query
    status of CapsLock, NumLock and ScrollLock keys.
]]--

ledKeys = Host.CreatePluginInstance( 'LedKeys' )
ledKeys:Connect( )

function Main( )
    if ledKeys:GetProperty( 'capsLock' ) then
        print( 'Caps Lock is ON' )
    end

    if ledKeys:GetProperty( 'numLock' ) then
        print( 'Num Lock is ON' )
    end

    if ledKeys:GetProperty( 'scrollLock' ) then
        print( 'Scroll Lock is ON' )
    end
end
