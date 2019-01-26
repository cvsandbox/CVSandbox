--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which demonstrates detection of 1D bar codes.
    No recognition, just detection. Each detected bar code
    is highlighted.
--]]

local table = require "table"
local math  = require "math"

-- Create instances of plug-ins to use
detectorPlugin = Host.CreatePluginInstance( 'BarCodeDetector' )
drawingPlugin  = Host.CreatePluginInstance( 'ImageDrawing' )

-- Colors used to highlight bar codes' corners
cornerColors = { 'FF0000', '880000', 'FFAAAA', 'FF8800' }

-- Color used for drawing
drawingColor = 'FF0000'

function Main( )
    image = Host.GetImage( )

    -- Find bar codes in the current image
    detectorPlugin:ProcessImage( image )

    barcodesFound = detectorPlugin:GetProperty( 'barcodesFound' )
    barcodeQuads  = detectorPlugin:GetProperty( 'barcodeQuadrilaterals' )

    -- Tell how many bar codes are detected
    drawingPlugin:CallFunction( 'DrawText', image, 'Found bar codes: ' .. tostring( barcodesFound ),
                                { 5, 5 }, drawingColor, '00000000' )

    -- Highlight each detected bar code
    for i = 1, barcodesFound do
        quad = barcodeQuads[i]

        DrawPolygon( image, quad )

        for j = 1, 4 do
            drawingPlugin:CallFunction( 'FillRectangle', image,
                                        { quad[j][1] - 3, quad[j][2] - 3 },
                                        { quad[j][1] + 3, quad[j][2] + 3 },
                                        cornerColors[j] )
        end

        -- Draw line crossing the bar code
        DrawLine( image,
                  { math.floor( ( quad[1][1] + quad[4][1] ) / 2 ),
                    math.floor( ( quad[1][2] + quad[4][2] ) / 2 ) },
                  { math.floor( ( quad[2][1] + quad[3][1] ) / 2 ),
                    math.floor( ( quad[2][2] + quad[3][2] ) / 2 ) } )
    end
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

        DrawLine( image, polygon[i], polygon[next] )
    end
end

-- Draw a thicker line using a filled polygon
function DrawLine( image, lineStart, lineEnd )

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
