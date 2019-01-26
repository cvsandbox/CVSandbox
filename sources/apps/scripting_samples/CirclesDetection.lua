--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which demonstrates detection of circles.
    It assumes objects are placed on a dark enough background,
    so that segmentation can be done using thresholding.

    Each detected circle is highlighted. Radius and mean
    deviation (from ideal shape) are provided for each
    circle.
--]]

local math   = require "math"
local string = require "string"

-- Create instances of plug-ins to use
grayscalePlugin     = Host.CreatePluginInstance( 'Grayscale' )
thresholdPlugin     = Host.CreatePluginInstance( 'Threshold' )
circlesFilterPlugin = Host.CreatePluginInstance( 'FilterCircleBlobs' )
drawingPlugin       = Host.CreatePluginInstance( 'ImageDrawing' )

-- Set threshold to separate background and objects
thresholdPlugin:SetProperty( 'threshold', 64 )

-- Don't do image filtering, only collect information about circles
circlesFilterPlugin:SetProperty( 'filterImage', false )
-- Set minimum radius of circles to collect
circlesFilterPlugin:SetProperty( 'minRadius', 5 )

-- Color used for drawing
drawingColor = '00FF00'

function Main( )
    image = Host.GetImage( )

    -- Pre-process image by grayscaling and thresholding it
    grayImage = grayscalePlugin:ProcessImage( image )
    thresholdPlugin:ProcessImageInPlace( grayImage )

    -- Apply circles filter
    circlesFilterPlugin:ProcessImageInPlace( grayImage )

    circlesFound    = circlesFilterPlugin:GetProperty( 'circlesFound' )
    circlesCenters  = circlesFilterPlugin:GetProperty( 'circlesCenters' )
    circlesRadiuses = circlesFilterPlugin:GetProperty( 'circlesRadiuses' )
    meanDeviations  = circlesFilterPlugin:GetProperty( 'meanDeviations' )

    -- Tell how many circles are detected
    drawingPlugin:CallFunction( 'DrawText', image, 'Circles: ' .. tostring( circlesFound ),
                                { 5, 5 }, drawingColor, '00000000' )

    -- Highlight each detected circle
    for i = 1, circlesFound do
        center = { math.floor( circlesCenters[i][1] ), math.floor( circlesCenters[i][2] ) }
        radius = math.floor( circlesRadiuses[i] )
        dist   = math.floor( math.sqrt( radius * radius / 2 ) )

        lineStart = { center[1] + radius, center[2] - radius }
        lineEnd   = { center[1] + dist, center[2] - dist }

        drawingPlugin:CallFunction( 'FillRing', image, center, radius + 2, radius, drawingColor )

        drawingPlugin:CallFunction( 'DrawLine', image, lineStart, lineEnd, drawingColor )
        drawingPlugin:CallFunction( 'DrawLine', image, lineStart, { lineStart[1] + 20, lineStart[2] }, drawingColor )

        -- Tell radius and mean deviation
        drawingPlugin:CallFunction( 'DrawText', image, tostring( radius ),
                                    { lineStart[1] + 2, lineStart[2] - 12 }, drawingColor, '00000000' )
        drawingPlugin:CallFunction( 'DrawText', image, string.format( '%.2f', meanDeviations[i] ),
                                    { lineStart[1] + 2, lineStart[2] + 3 }, drawingColor, '00000000' )
    end

    grayImage:Release( )
end
