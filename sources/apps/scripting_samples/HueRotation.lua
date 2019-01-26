--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    This a simple "Hello World" sample demonstrating
    basics of using an image processing plug-in from
    a Lua script.
--]]

-- Create instance of Colorize plug-in
setHuePlugin  = Host.CreatePluginInstance( 'Colorize' )
-- Set Saturation to maximum
setHuePlugin:SetProperty( 'saturation', 100 )
-- Start with Hue set to 0
hue = 0

-- Main function to be executed for every frame
function Main( )
    -- Get image to process
    image = Host.GetImage( )
    -- Set Hue value to set for the image
    setHuePlugin:SetProperty( 'hue', hue )
    -- Process the image
    setHuePlugin:ProcessImageInPlace( image )
    -- Move to the next Hue value
    hue = ( hue + 1 ) % 360
end
