--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which demonstrates detection of rectangles.
    It assumes objects are placed on a dark enough background,
    so that segmentation can be done using thresholding.

    Each detected rectangle is highlighted and length of its
    sides is provided.
--]]

local table = require "table"
local math  = require "math"

-- Create instances of plug-ins to use
grayscalePlugin   = Host.CreatePluginInstance( 'Grayscale' )
thresholdPlugin   = Host.CreatePluginInstance( 'Threshold' )
quadsFilterPlugin = Host.CreatePluginInstance( 'FilterQuadrilateralBlobs' )
drawingPlugin     = Host.CreatePluginInstance( 'ImageDrawing' )

-- Set threshold to separate background and objects
thresholdPlugin:SetProperty( 'threshold', 48 )

-- Don't do filtering image, only collect information about rectangles
quadsFilterPlugin:SetProperty( 'filterImage', false )
-- Keep rectangles only (which includes squares)
quadsFilterPlugin:SetProperty( 'quadrilateralsToKeep', 1 )
-- Minimum/Maximum allowed length of rectangles' sides
quadsFilterPlugin:SetProperty( 'minSize', 10 )
quadsFilterPlugin:SetProperty( 'maxSize', 200 )
-- Relative distortion in %
quadsFilterPlugin:SetProperty( 'relDistortion', 7 )
-- Minimum allowed mean distortion
quadsFilterPlugin:SetProperty( 'minDistortion', 4 )

-- Colors used to highlight rectangles' corners
cornerColors = { '00FF00', '008800', 'AAFFAA', '00FFFF' }

-- Color used for drawing
drawingColor = '00FF00'

function Main( )
    image = Host.GetImage( )

    -- Pre-process image by grayscaling and thresholding it
    grayImage = grayscalePlugin:ProcessImage( image )
    thresholdPlugin:ProcessImageInPlace( grayImage )

    -- Apply quadrilateral filter
    quadsFilterPlugin:ProcessImageInPlace( grayImage )

    quadsFound = quadsFilterPlugin:GetProperty( 'quadrilateralsFound' )
    quads      = quadsFilterPlugin:GetProperty( 'quadrilaterals' )

    -- Tell how many rectangles are detected
    drawingPlugin:CallFunction( 'DrawText', image, 'Rectangles: ' .. tostring( quadsFound ),
                                { 5, 5 }, drawingColor, '00000000' )

    -- Highlight each detected rectangle
    for i = 1, quadsFound do
        quad = quads[i]
        maxY = 0
        minX = image:Width( )

        DrawPolygon( image, quad )

        -- Four corners of each rectangle
        for j = 1, 4 do
            drawingPlugin:CallFunction( 'FillRectangle', image,
                                        { quad[j][1] - 3, quad[j][2] - 3 },
                                        { quad[j][1] + 3, quad[j][2] + 3 },
                                        cornerColors[j] )

            if ( quad[j][2] > maxY ) then
                maxY = quad[j][2]
            end
            if ( quad[j][1] < minX ) then
                minX = quad[j][1]
            end
        end

        -- Calculate length of rectangle's both sides
        dx1 = quad[2][1] - quad[1][1]
        dy1 = quad[2][2] - quad[1][2]

        dx2 = quad[3][1] - quad[2][1]
        dy2 = quad[3][2] - quad[2][2]

        len1 = math.floor( math.sqrt( dx1 * dx1 + dy1 * dy1 ) )
        len2 = math.floor( math.sqrt( dx2 * dx2 + dy2 * dy2 ) )

        if ( len2 > len1 ) then
            temp = len1
            len1 = len2
            len2 = temp
        end

        strLen = tostring( len1 ) .. ' / ' .. tostring( len2 )
        drawingPlugin:CallFunction( 'DrawText', image, strLen, { minX, maxY + 4 }, drawingColor, '00000000' )
    end

    grayImage:Release( )
end

-- Both 'DrawLine' and 'DrawPolyline' functions of image drawing plug-in
-- don't support line thickness, so using this function as a work around
function DrawPolygon( image, polygon )

    cornersCount = #polygon

    for i = 1, cornersCount do
        next = i + 1
        if ( next > cornersCount ) then
            next = 1
        end

        lineStart  = polygon[i]
        lineEnd    = polygon[next]
        dx         = lineEnd[1] - lineStart[1]
        dy         = lineEnd[2] - lineStart[2]
        sidePoints = {}

        if ( dx < 0 ) then dx = -dx end
        if ( dy < 0 ) then dy = -dy end

        if ( dx > dy ) then
            -- If the line is more horizontal, then extend it in Y direction
            sidePoints[1] = { lineStart[1], lineStart[2] - 1 }
            sidePoints[2] = { lineEnd[1], lineEnd[2] - 1 }
            sidePoints[3] = { lineEnd[1], lineEnd[2] + 1 }
            sidePoints[4] = { lineStart[1], lineStart[2] + 1 }
        else
            -- Otherwise extend it in X direction
            sidePoints[1] = { lineStart[1] - 1, lineStart[2] }
            sidePoints[2] = { lineStart[1] + 1, lineStart[2] }
            sidePoints[3] = { lineEnd[1] + 1, lineEnd[2] }
            sidePoints[4] = { lineEnd[1] - 1, lineEnd[2] }
        end

        drawingPlugin:CallFunction( 'FillConvexPolygon', image, sidePoints, drawingColor )
    end
end
