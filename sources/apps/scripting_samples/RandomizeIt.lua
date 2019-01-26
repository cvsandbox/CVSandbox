--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    A sample script which uses a variety of image processing plug-ins to
    create sort of unstable environment, where many different things
    change like: brightness, contrast, saturation, noise level, shaking
    (rotation), etc. This creates sort of old/shaking movie.
--]]

-- Require Lua's math module
local math = require "math"

-- Create instances of plug-ins to use
brightnessPlugin = Host.CreatePluginInstance( 'BrightnessCorrection' )
contrastPlugin   = Host.CreatePluginInstance( 'ContrastCorrection' )
saturatePlugin   = Host.CreatePluginInstance( 'Saturate' )
noisePlugin      = Host.CreatePluginInstance( 'UniformAdditiveNoise' )
rotatePlugin     = Host.CreatePluginInstance( 'RotateImage' )
borderPlugin     = Host.CreatePluginInstance( 'RoundedBorder' )

-- Start values of some properties
brightnessFactor = 0
contrastFactor   = 0
saturateBy       = 0
noiseAmplitude   = 20
rotateAngle      = 0
borderWidth      = 32

borderPlugin:SetProperty( 'borderColor', '000000' )

-- Main function to be executed for every frame
function Main( )
    -- Randomize some properties of the plug-ins in use
    RandomizeIt( )
    -- Get image to process
    image = Host.GetImage( )
    -- Apply image processing routines
    brightnessPlugin:ProcessImageInPlace( image )
    contrastPlugin:ProcessImageInPlace( image )
    saturatePlugin:ProcessImageInPlace( image )
    noisePlugin:ProcessImageInPlace( image )
    newImage = rotatePlugin:ProcessImage( image )
    borderPlugin:ProcessImageInPlace( newImage )
    -- Set the new image back to host
    Host.SetImage( newImage )
    -- Release the new image
    newImage:Release( )
end

-- Make sure the specified value is in the specified range
function CheckRange( value, min, max )
    if value < min then value = min end
    if value > max then value = max end
    return value
end

-- Modify plug-ins' properties randomly
function RandomizeIt( )
    -- change brightness factor
    brightnessFactor = CheckRange( brightnessFactor + math.random( ) / 5 - 0.1, -0.5, 0.5 )
    brightnessPlugin:SetProperty( 'factor', brightnessFactor )
    -- change contrast factor
    contrastFactor = CheckRange( contrastFactor + math.random( ) / 5 - 0.1, -0.5, 0.5 )
    contrastPlugin:SetProperty( 'factor', contrastFactor )
    -- change saturation
    saturateBy = CheckRange( saturateBy + math.random( 5 ) - 3, -20, 20 )
    saturatePlugin:SetProperty( 'saturateBy', saturateBy )
    -- change noise level
    noiseAmplitude = CheckRange( noiseAmplitude + math.random( 5 ) - 3, 10, 60 )
    noisePlugin:SetProperty( 'amplitude', noiseAmplitude )
    -- change rotation angle
    rotateAngle = CheckRange( rotateAngle + math.random( ) - 0.5, -5, 5 )
    rotatePlugin:SetProperty( 'angle', rotateAngle )
    -- change border width
    borderWidth = CheckRange( borderWidth + math.random( 3 ) - 2, 30, 35 )
    borderPlugin:SetProperty( 'borderWidth', borderWidth )
end
