--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    The script demonstrates how to combine JPEG files from the
    specified folder with incoming video frames.
--]]

-- Make sure we have required APIs exposed
if SCRIPTING_API_REVISION == nil then
    error( 'The script requires minimum API revision 3' )
end

local io    = require "io"
local math  = require "math"

-- Folder to look for *.jpg files in (terminate it with \)
folder  = [[C:\Pictures\Browser\]]

-- Get the list of files to browse (works on Window only)
prog    = 'dir "' .. folder .. '*.jpg" /b /a-d'
filesIt = io.popen( prog ):lines( )

-- Build array out of iterator function
files = { }
for file in filesIt do
    files[#files + 1] = file
end
filesCount = #files

-- Check number of files
if filesCount == 0 then
    error( "Did not find any JPEG files (*.jpg) in the specified folder" )
end

-- Create instances of plug-ins to use
jpegImporterPlugin = Host.CreatePluginInstance( 'JpegImporter' )
grayToRgbPlugin    = Host.CreatePluginInstance( 'GrayscaleToRgb' )
resizeImagePlugin  = Host.CreatePluginInstance( 'ResizeImage' )
ledKeysPlugin      = Host.CreatePluginInstance( 'LedKeys' )
drawing            = Host.CreatePluginInstance( 'ImageDrawing' )

-- Set bilinear interpolation (1). Nearest neighbour (0) can be also used to make it faster,
-- but reduces quality
resizeImagePlugin:SetProperty( 'interpolation', 1 )

lastWidth   = 0
lastHeight  = 0
currentFile = 1

-- Loaded image and its resized version
loadedImage        = nil
resizedLoadedImage = nil

-- Connect the LedKeys device plug-in and get NumLock status
ledKeysPlugin:Connect( )
prevNumLockState = ledKeysPlugin:GetProperty( 'numLock' )

-- Main function to be executed for every frame
function Main( )
    -- Get image to process
    image = Host.GetImage( )

    width  = image:Width( )
    height = image:Height( )

    smallWidth  = width  / 4;
    smallHeight = height / 4;

    if ledKeysPlugin:GetProperty( 'capsLock' ) then

        -- Check if we need to switch to another image
        if prevNumLockState ~= ledKeysPlugin:GetProperty( 'numLock' ) then
            currentFile = currentFile + 1
            if currentFile > filesCount then
                currentFile = 1
            end

            if loadedImage ~= nil then
                loadedImage:Release( )
                loadedImage = nil
            end

            if resizedLoadedImage ~= nil then
                resizedLoadedImage:Release( )
                resizedLoadedImage = nil
            end
        end

        prevNumLockState = ledKeysPlugin:GetProperty( 'numLock' )

        -- Load image if required
        if loadedImage == nil then
            loadedImage = jpegImporterPlugin:ImportImage( folder .. files[currentFile] )

            -- If we got grayscale JPEG, convert it to RGB
            if loadedImage:PixelFormat( ) == 'Gray8' then
                tempImage = grayToRgbPlugin:ProcessImage( loadedImage )
                loadedImage:Release( )
                loadedImage = tempImage
            end
        end

        -- Make sure video size did not change
        if ( lastWidth ~= width ) or ( lastHeight ~= height ) then
            if resizedLoadedImage ~= nil then
                resizedLoadedImage:Release( )
                resizedLoadedImage = nil
            end
        end

        -- Determine the size to resize loaded image to
        resizeWidth  = smallWidth
        resizeHeight = smallHeight

        if ledKeysPlugin:GetProperty( 'scrollLock' ) then
            resizeWidth  = width
            resizeHeight = height
        end

        -- Preserve aspect ratio
        resizeAspect = resizeWidth / resizeHeight
        loadedAspect = loadedImage:Width( ) / loadedImage:Height( )

        if ( resizeApsect ~= loadedAspect ) then
            if ( loadedAspect > resizeAspect ) then
                resizeHeight = math.floor( resizeWidth / loadedAspect )
            else
                resizeWidth = math.floor( resizeHeight * loadedAspect )
            end
        end

        -- Release any image of the wrong size
        if ( resizedLoadedImage ~= nil ) and
           ( ( resizeWidth ~= resizedLoadedImage:Width( ) ) or ( resizeHeight ~= resizedLoadedImage:Height( ) ) ) then

            resizedLoadedImage:Release( )
            resizedLoadedImage = nil
        end

        -- Create new resized image
        if resizedLoadedImage == nil then
            resizeImagePlugin:SetProperty( 'width',  resizeWidth )
            resizeImagePlugin:SetProperty( 'height', resizeHeight )
            resizedLoadedImage = resizeImagePlugin:ProcessImage( loadedImage )
        end

        if ledKeysPlugin:GetProperty( 'scrollLock' ) == false then
            -- Just put resized loaded image in the top-right corner
            image:PutImage( resizedLoadedImage, width - resizeWidth, 0 )
        else
            -- Resize video frame, which goes to the top-right corner
            resizeImagePlugin:SetProperty( 'width',  smallWidth )
            resizeImagePlugin:SetProperty( 'height', smallHeight )
            resizedVideo = resizeImagePlugin:ProcessImage( image )

            if ( ( resizeWidth ~= width ) or ( resizeHeight ~= height ) ) then
                -- The resized image is smaller than original video size,
                -- so fill the gaps
                if ( resizeWidth < width ) then
                    drawing:CallFunction( 'FillRectangle', image, { resizeWidth, 0 }, { width - 1, height - 1 }, '000000' )
                else
                    drawing:CallFunction( 'FillRectangle', image, { 0, resizeHeight }, { width - 1, height - 1 }, '000000' )
                end
            end

            -- Finally put the resized picture and resize video on top
            image:PutImage( resizedLoadedImage, 0, 0 )
            image:PutImage( resizedVideo, width - smallWidth, 0 )

            -- Release the resized video frame
            resizedVideo:Release( )
        end

        lastWidth  = width
        lastHeight = height
    end
end
