--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    A sample script which allows switching between different effects
    (taken from other samples) by pressing CapsLock, NumLock and ScrolLock.
--]]

-- Require Lua's math module
local math = require "math"

-- Create instances of plug-ins to use
pixellatePlugin  = Host.CreatePluginInstance( 'Pixellate' )

brightnessPlugin  = Host.CreatePluginInstance( 'BrightnessCorrection' )
contrastPlugin    = Host.CreatePluginInstance( 'ContrastCorrection' )
saturatePlugin    = Host.CreatePluginInstance( 'Saturate' )
noisePlugin       = Host.CreatePluginInstance( 'UniformAdditiveNoise' )
rotatePlugin      = Host.CreatePluginInstance( 'RotateImage' )
borderPlugin      = Host.CreatePluginInstance( 'RoundedBorder' )

sepiaPlugin       = Host.CreatePluginInstance( 'Sepia' )
vignettingPlugin  = Host.CreatePluginInstance( 'Vignetting' )
grainPlugin       = Host.CreatePluginInstance( 'Grain' )
fuzzyBorderPlugin = Host.CreatePluginInstance( 'FuzzyBorder' )

ledKeysPlugin     = Host.CreatePluginInstance( 'LedKeys' )

-- Start values of some properties
pixelWidth       = 1
pixelHeight      = 1

brightnessFactor = 0
contrastFactor   = 0
saturateBy       = 0
noiseAmplitude   = 20
rotateAngle      = 0
borderWidth      = 32

vignettingStartFactor = 80
vignettingEndFactor   = 140
grainSpacing          = 40
noiseAmplitude        = 10

borderPlugin:SetProperty( 'borderColor', '000000' )

vignettingPlugin:SetProperty( 'decreaseSaturation', false )
vignettingPlugin:SetProperty( 'startFactor', vignettingStartFactor )
vignettingPlugin:SetProperty( 'endFactor', vignettingEndFactor )
grainPlugin:SetProperty( 'staticSeed', true )
grainPlugin:SetProperty( 'density', 0.5 )
fuzzyBorderPlugin:SetProperty( 'borderColor', '000000' )
fuzzyBorderPlugin:SetProperty( 'borderWidth', 32 )
fuzzyBorderPlugin:SetProperty( 'waviness', 8 )
fuzzyBorderPlugin:SetProperty( 'gradientWidth', 16 )

-- Connect LedKeys plug-in, so we can query status of CapsLock, NumLock and ScrolLock
ledKeysPlugin:Connect( )

-- Other variables
counter      = 0
increasing   = true
maxPixelSize = 16
seed         = 0

-- Main function to be executed for every frame
function Main( )

    -- Get image to process
    image = Host.GetImage( )

    -- Decide which effect to use
    if ledKeysPlugin:GetProperty( 'capsLock' ) then
        newImage = Effect1( image )
    elseif ledKeysPlugin:GetProperty( 'numLock' ) then
        newImage = Effect2( image )
    elseif ledKeysPlugin:GetProperty( 'scrollLock' ) then
        newImage = Effect3( image )
    else
        newImage = image
    end

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

-- Perform 1st image processing effect
function Effect1( image )

    pixellatePlugin:ProcessImageInPlace( image )

    -- Update properties
    if counter == 0 then
        if increasing then
            -- increase pixel width first, then height
            if pixelWidth == maxPixelSize then
                pixelHeight = pixelHeight + 1
                if pixelHeight == maxPixelSize then
                    increasing = false
                end
            else
                pixelWidth = pixelWidth + 1
            end
        else
            -- decrease pixel width first, then height
            if pixelWidth == 1 then
                pixelHeight = pixelHeight - 1
                if pixelHeight == 1 then
                    increasing = true
                end
            else
                pixelWidth = pixelWidth - 1
            end
        end

        pixellatePlugin:SetProperty( 'pixelWidth', pixelWidth )
        pixellatePlugin:SetProperty( 'pixelHeight', pixelHeight )
    end

    counter = ( counter + 1 ) % 5

    return image
end

-- Perform 2nd image processing effect
function Effect2( image )
    -- Randomize some properties of the plug-ins in use
    RandomizeEffect2( )
    -- Apply image processing routines
    brightnessPlugin:ProcessImageInPlace( image )
    contrastPlugin:ProcessImageInPlace( image )
    saturatePlugin:ProcessImageInPlace( image )
    noisePlugin:ProcessImageInPlace( image )
    newImage = rotatePlugin:ProcessImage( image )
    borderPlugin:ProcessImageInPlace( newImage )

    return newImage
end

-- Perform 3rd image processing effect
function Effect3( image )
    -- Randomize some properties of the plug-ins in use
    RandomizeEffect3( )
    -- Apply image processing routines
    sepiaPlugin:ProcessImageInPlace( image )
    vignettingPlugin:ProcessImageInPlace( image )
    grainPlugin:ProcessImageInPlace( image )
    noisePlugin:ProcessImageInPlace( image )
    fuzzyBorderPlugin:ProcessImageInPlace( image )

    return image
end

-- Modify plug-ins' properties randomly for the 2nd effect
function RandomizeEffect2( )
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

-- Modify plug-ins' properties randomly for the 3rd effect
function RandomizeEffect3( )
    -- change vignetting start/end factors
    vignettingStartFactor = CheckRange( vignettingStartFactor + math.random( 3 ) - 2, 70, 100 )
    vignettingPlugin:SetProperty( 'startFactor', vignettingStartFactor )
    vignettingEndFactor = CheckRange( vignettingEndFactor + math.random( 3 ) - 2, 120, 150 )
    vignettingPlugin:SetProperty( 'endFactor', vignettingEndFactor )

    -- change noise level
    noiseAmplitude = CheckRange( noiseAmplitude + math.random( 5 ) - 3, 10, 30 )
    noisePlugin:SetProperty( 'amplitude', noiseAmplitude )

    -- change grain and every 5th frame
    counter = ( counter + 1 ) % 5
    if counter == 0 then
        -- grain's seed value
        seed = seed + 1
        grainPlugin:SetProperty( 'seedValue', seed )
        -- grain's spacing
        grainSpacing = CheckRange( grainSpacing + math.random( 5 ) - 3, 30, 50 )
    end
end
