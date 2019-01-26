--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    The script demonstrates saving of video frames into
    image files at a certain time intervals.
--]]

local os = require "os"

-- Folder to write images to
folder = 'C:\\Temp\\images\\'

-- Create instance of plug-in for saving images
imageWriter = Host.CreatePluginInstance( 'PngExporter' )
--imageWriter = Host.CreatePluginInstance( 'JpegExporter' )
--imageWriter:SetProperty( 'quality', 100 )
ext = '.' .. imageWriter:SupportedExtensions( )[1]

-- Interval between images in seconds
imageInterval = 10
lastClock     = -imageInterval

function Main( )
    image = Host.GetImage( )

    -- Get number of seconds the application is running
    now = os.clock( )

    if now - lastClock >= imageInterval then
        lastClock = now
        SaveImage( image )
    end
end

-- Save image to file
function SaveImage( image )
    dateTime = os.date( '%Y-%m-%d %H-%M-%S' )
    fileName = folder .. dateTime .. ext
    imageWriter:ExportImage( fileName, image )
end
