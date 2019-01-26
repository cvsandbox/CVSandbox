--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    The script demonstrates different functions to draw on images.
--]]

local math = require "math"

drawing = Host.CreatePluginInstance( 'ImageDrawing' )

color       = 'FF00FF00'
transparent = '00000000'
fillColor1  = '6000FF00'
fillColor2  = 'B000FF00'

progress = 0
counter  = 0

function Main( )
    image = Host.GetImage( )

    width  = image:Width( )
    height = image:Height( )
    cx     = math.floor( width  / 2 )
    cy     = math.floor( height / 2 )

    -- Draw a circle in the center
    drawing:CallFunction( 'DrawCircle', image, { cx, cy }, height / 3, color )

    -- Draw a cross in the center
    drawing:CallFunction( 'DrawLine', image, { cx - 5, cy }, { cx + 5, cy }, color )
    drawing:CallFunction( 'DrawLine', image, { cx, cy - 5 }, { cx, cy + 5 }, color )

    -- Draw a progress bar
    drawing:CallFunction( 'DrawRectangle', image,
        { cx - 100, height - 30 }, { cx + 100, height - 15 }, color )
    drawing:CallFunction( 'FillRectangle', image,
        { cx + 36, height - 28 }, { cx + 98, height - 17 }, fillColor1 )
    drawing:CallFunction( 'FillRectangle', image,
        { cx - 98, height - 28 }, { cx + 35, height - 17 }, fillColor2 )

    -- Draw rulers on the left and right
    startY = math.floor( height / 6 )
    steps  = 21
    stepY  = math.floor( ( height / 3 ) * 2 / steps )
    for i = 1, steps do
        y   = startY + ( i - 1 ) * stepY
        len = 5 + 5 * ( i % 2 )
        drawing:CallFunction( 'DrawLine', image, { 5, y }, { 5 + len, y }, color )
        drawing:CallFunction( 'DrawLine', image, { width - 5, y }, { width - 5 - len, y }, color )
    end

    drawing:CallFunction( 'DrawLine', image, { 5, startY }, { 5, startY + ( steps - 1 ) * stepY }, color )
    drawing:CallFunction( 'DrawLine', image, { width - 5, startY }, { width - 5, startY + ( steps - 1 ) * stepY }, color )

    -- Draw another ruler at the top
    startX = math.floor( width / 6 )
    steps  = 21
    stepX  = math.floor( ( width / 3 ) * 2 / steps )
    for i = 1, steps do
        x   = startX + ( i - 1 ) * stepX
        len = -5 + 10 * ( i % 2 )
        drawing:CallFunction( 'DrawLine', image, { x, 10 }, { x, 10 + len }, color )
    end
    drawing:CallFunction( 'DrawLine', image, { startX, 10}, { startX + ( steps - 1 ) * stepX, 10 }, fillColor1 )

    -- Draw 4 corners
    dx = math.floor( width  / 5 * 2 )
    dy = math.floor( height / 5 * 2 )

    DrawCorner( image, cx - dx, cy - dy,  5,  5 )
    DrawCorner( image, cx - dx, cy + dy,  5, -5 )
    DrawCorner( image, cx + dx, cy - dy, -5,  5 )
    DrawCorner( image, cx + dx, cy + dy, -5, -5 )

    -- Show circle progress in the left corner
    r1 = math.floor( height / 10 )
    r2 = r1 - 10
    a  = math.floor( progress * 3.6 )
    strProgress = tostring( progress )

    drawing:CallFunction( 'FillRing', image, { cx - dx + r1, cy + dy - r1 }, r1, r2, fillColor1 )
    drawing:CallFunction( 'DrawText', image, strProgress,
                          { cx - dx + r1 - ( #strProgress * 4 ), cy + dy - r1 - 4}, color, transparent )
    if progress ~= 0 then
        drawing:CallFunction( 'FillPie', image, { cx - dx + r1, cy + dy - r1 }, r1 - 2, r2 + 2, 0, a, color )
    end

    -- Rotate triangle in the left corner
    r3 = r2 - 2

    triangle =
    {
        {
            cx + dx - r1 + math.floor( r3 * math.cos( math.rad( a ) ) ),
            cy + dy - r1 + math.floor( r3 * math.sin( math.rad( a ) ) )
        },
        {
            cx + dx - r1 + math.floor( r3 * math.cos( math.rad( a + 120 ) ) ),
            cy + dy - r1 + math.floor( r3 * math.sin( math.rad( a + 120 ) ) )
        },
        {
            cx + dx - r1 + math.floor( r3 * math.cos( math.rad( a + 240 ) ) ),
            cy + dy - r1 + math.floor( r3 * math.sin( math.rad( a + 240 ) ) )
        }
    }
    drawing:CallFunction( 'FillConvexPolygon', image, triangle, fillColor1 )

    drawing:CallFunction( 'FillPie', image, { cx + dx - r1, cy + dy - r1 }, r1 - 2, r2 + 2, a, a - 5, fillColor2 )

    -- Draw a ruler in the center
    steps  = 2
    stepY  = 60
    length = math.floor( height / 5 )
    stepX  = math.floor( length / 3 )

    for i = 0, steps do
        leftX1 = cx - stepX
        leftX2 = cx - stepX - length

        rightX1 = cx + stepX
        rightX2 = cx + stepX + length

        y1 = cy + i * stepY
        y2 = cy - i * stepY

        label1 = tostring(  i * 5 )
        label2 = tostring( -i * 5 )

        drawing:CallFunction( 'DrawLine', image, { leftX1, y1 }, { leftX2, y1 }, color )
        drawing:CallFunction( 'DrawText', image, label1, { leftX2 - 2 - 8 * #label1, y1 - 4 }, color, transparent )

        drawing:CallFunction( 'DrawLine', image, { rightX1, y1 }, { rightX2, y1 }, color )
        drawing:CallFunction( 'DrawText', image, label1, { rightX2 + 2, y1 - 4 }, color, transparent )

        if i ~= 0 then
            drawing:CallFunction( 'DrawLine', image, { leftX1, y2 }, { leftX2, y2 }, color )
            drawing:CallFunction( 'DrawText', image, label2, { leftX2 - 2 - 8 * #label2, y2 - 4 }, color, transparent )

            drawing:CallFunction( 'DrawLine', image, { rightX1, y2 }, { rightX2, y2 }, color )
            drawing:CallFunction( 'DrawText', image, label2, { rightX2 + 2, y2 - 4 }, color, transparent )
        end
    end

    --
    counter = ( counter + 1 ) % 5
    if counter == 0 then
        progress = ( progress + 1 ) % 101
    end
end

function DrawCorner( image, x, y, dx, dy )
    drawing:CallFunction( 'DrawLine', image, { x, y }, { x + dx, y }, color )
    drawing:CallFunction( 'DrawLine', image, { x, y }, { x, y + dy }, color )
end