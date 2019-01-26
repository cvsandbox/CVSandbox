--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    A sample script which uses a variety of image processing plug-ins to
    create an old film effect, which contains random noise, vertical grain
    and changing vignetting effect. This all is displayed using sepia
    colors and surrounded with a fuzzy border.
--]]

-- Require Lua's math module
local math = require "math"

-- Create instances of plug-ins to use
sepiaPlugin      = Host.CreatePluginInstance( 'Sepia' )
vignettingPlugin = Host.CreatePluginInstance( 'Vignetting' )
grainPlugin      = Host.CreatePluginInstance( 'Grain' )
noisePlugin      = Host.CreatePluginInstance( 'UniformAdditiveNoise' )
borderPlugin     = Host.CreatePluginInstance( 'FuzzyBorder' )

-- Start values of some properties
vignettingStartFactor = 80
grainSpacing          = 40
noiseAmplitude        = 20

vignettingPlugin:SetProperty( 'decreaseSaturation', false )
vignettingPlugin:SetProperty( 'startFactor', vignettingStartFactor )
vignettingPlugin:SetProperty( 'endFactor', 150 )
grainPlugin:SetProperty( 'staticSeed', true )
grainPlugin:SetProperty( 'density', 0.5 )
borderPlugin:SetProperty( 'borderColor', '000000' )
borderPlugin:SetProperty( 'borderWidth', 32 )
borderPlugin:SetProperty( 'waviness', 8 )
borderPlugin:SetProperty( 'gradientWidth', 16 )

-- Other variables
seed    = 0
counter = 0

-- Main function to be executed for every frame
function Main( )
    -- Randomize some properties of the plug-ins in use
    RandomizeIt( )
    -- Get image to process
    image = Host.GetImage( )
    -- Apply image processing routines
    sepiaPlugin:ProcessImageInPlace( image )
    vignettingPlugin:ProcessImageInPlace( image )
    grainPlugin:ProcessImageInPlace( image )
    noisePlugin:ProcessImageInPlace( image )
    borderPlugin:ProcessImageInPlace( image )
end

-- Make sure the specified value is in the specified range
function CheckRange( value, min, max )
    if value < min then value = min end
    if value > max then value = max end
    return value
end

-- Modify plug-ins' properties randomly
function RandomizeIt( )
    -- change vignetting start factor
    vignettingStartFactor = CheckRange( vignettingStartFactor + math.random( 3 ) - 2, 60, 100 )
    vignettingPlugin:SetProperty( 'startFactor', vignettingStartFactor )
    -- change noise level
    noiseAmplitude = CheckRange( noiseAmplitude + math.random( 5 ) - 3, 10, 30 )
    noisePlugin:SetProperty( 'amplitude', noiseAmplitude )

    -- change grain every 5th frame
    counter = ( counter + 1 ) % 5
    if counter == 0 then
        -- grain's seed value
        seed = seed + 1
        grainPlugin:SetProperty( 'seedValue', seed )
        -- grain's spacing
        grainSpacing = CheckRange( grainSpacing + math.random( 5 ) - 3, 30, 50 )
    end
end
