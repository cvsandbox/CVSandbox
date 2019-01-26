--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which uses two plug-ins: one which supports
    in-place image processing and another which does not. And
    so the new image must be set back to the host application
    for further processing or display.

    It changes Emboss plug-in's "azimuth" property, which is
    direction of a light source, and "hue" property of the
    Colorize plug-in's at the same time.
--]]

-- Create instances of plug-ins to use
embossPlugin   = Host.CreatePluginInstance( 'Emboss' )
colorizePlugin = Host.CreatePluginInstance( 'Colorize' )

-- Start values of some properties
azimuth = 0
hue     = 0

embossPlugin:SetProperty( 'elevation', 45 )
embossPlugin:SetProperty( 'depth', 5 )
colorizePlugin:SetProperty( 'saturation', 70 )

-- Main function to be executed for every frame
function Main( )
    -- Get image to process
    image = Host.GetImage( )
    -- Apply image processing routines
    newImage = embossPlugin:ProcessImage( image )
    colorizePlugin:ProcessImageInPlace( newImage )
    -- Set new image back to host
    Host.SetImage( newImage )

    -- Modify plug-ins' properties
    azimuth = ( azimuth + 1 ) % 360
    hue     = ( hue     + 1 ) % 360

    embossPlugin:SetProperty( 'azimuth', azimuth )
    colorizePlugin:SetProperty( 'hue', hue )

    -- Release the new image
    newImage:Release( )
end
