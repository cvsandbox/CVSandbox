--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    Sample script which demonstrates detection of square binary glyphs.

    Each detected glyph is highlighted and its code is provided.
--]]

local table = require "table"

-- Create instances of plug-ins to use
detectorPlugin = Host.CreatePluginInstance( 'GlyphDetector' )
drawingPlugin  = Host.CreatePluginInstance( 'ImageDrawing' )

-- Detect up to 7 glyphs
detectorPlugin:SetProperty( 'maxGlyphs', 7 )

-- Colors used to highlight glyphs' corners
cornerColors = { 'FF0000', '880000', 'FFAAAA', 'FF8800' }

-- Color used for any other drawing
drawingColor = 'FF0000'

function Main( )
    image = Host.GetImage( )

    -- Find glyphs in the current image
    detectorPlugin:ProcessImage( image )

    glyphsFound    = detectorPlugin:GetProperty( 'glyphsFound' )
    glyphQuads     = detectorPlugin:GetProperty( 'glyphQuadrilaterals' )
    glyphCodes     = detectorPlugin:GetProperty( 'glyphCodes' )
    glyphPositions = detectorPlugin:GetProperty( 'glyphRectanglePosition' )
    glyphSizes     = detectorPlugin:GetProperty( 'glyphRectangleSize' )

    -- Tell how many glyphs are detected
    drawingPlugin:CallFunction( 'DrawText', image, 'Found glyphs: ' .. tostring( glyphsFound ),
                                { 5, 5 }, drawingColor, '00000000' )

    -- Highlight each detected glyph
    for i = 1, glyphsFound do
        quad = glyphQuads[i]
        rightMostPoint = quad[1]

        DrawPolygon( image, quad )

        -- Four corners of each rectangle
        for j = 1, 4 do
            drawingPlugin:CallFunction( 'FillRectangle', image,
                                        { quad[j][1] - 3, quad[j][2] - 3 },
                                        { quad[j][1] + 3, quad[j][2] + 3 },
                                        cornerColors[j] )

            if quad[j][1] > rightMostPoint[1] then
                rightMostPoint = quad[j]
            end
        end

        -- Tell the code of the glyph
        drawingPlugin:CallFunction( 'DrawText', image, glyphCodes[i],
                                    { rightMostPoint[1] + 4, rightMostPoint[2] },
                                    drawingColor, '00000000' )
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
