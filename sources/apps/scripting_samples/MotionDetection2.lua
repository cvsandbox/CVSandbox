--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    The script demonstrates implementation of a simple
    motion detector based on difference between current video
    frame and background frame. Initially, the first received
    video frame is taken as background frame. But then it slowly
    gets updated to decrease difference with new coming frames.
    The idea is that moving objects should leave scene quick
    enough before they appear on the background frame, while
    the rest is static enough and so gets into the background.

    The script also saves all detected motion into video files.
--]]

local os = require "os"

-- Create instances of plug-ins to use
diffImages   = Host.CreatePluginInstance( 'DiffImagesThresholded' )
addImages    = Host.CreatePluginInstance( 'AddImages' )
moveTowards  = Host.CreatePluginInstance( 'MoveTowardsImages' )
imageDrawing = Host.CreatePluginInstance( 'ImageDrawing' )
videoWriter  = Host.CreatePluginInstance( 'VideoFileWriter' )

-- Since we deal with RGB images, set threshold to 60 for the sum
-- of RGB differences
diffImages:SetProperty( 'threshold', 60 )
-- Highlight motion area with red color
diffImages:SetProperty( 'hiColor', 'FF0000' )

-- Amount of diff image to add to source image for motion highlighting
addImages:SetProperty( 'factor', 0.3 )

-- Motion alarm threshold
motionThreshold = 0.1

-- Update background after every 5th frame.
backgroundUpdateInterval = 5
counter = 0

-- Configure video file writer
videoWriter:SetProperty( 'folder', 'C:\\Temp\\camera' )
videoWriter:SetProperty( 'bitRate', 2000  )
videoWriter:SetProperty( 'syncPresentationTime', true )

-- Number of seconds to keep writing video after motion was detected
extraSaveTime = 5
-- Motion detection time (system clock, seconds)
motionDetectionTime = 0

-- Highlight motion areas or not
highlightMotion = true

videoFileStarted = false

function Main( )
    image = Host.GetImage( )

    if backgroundImage ~= nil then
        -- Calculate difference between current frame and the background
        diff = diffImages:ProcessImage( image, backgroundImage )

        -- Update background frame
        counter = ( counter + 1 ) % backgroundUpdateInterval
        if counter == 0 then
            moveTowards:ProcessImageInPlace( backgroundImage, image )
        end

        -- Get the difference amount
        diffPixels  = diffImages:GetProperty( 'diffPixels' )
        diffPercent = diffPixels * 100 / ( image:Width( ) * image:Height( ) )

        -- Output the difference value
        imageDrawing:CallFunction( 'DrawText', image, tostring( diffPercent ),
                                   { 1, 1 }, 'FFFFFF', '00000000' )

        -- Check if alarm has to be raised
        if diffPercent > motionThreshold then
            imageDrawing:CallFunction( 'DrawRectangle', image,
                { 0, 0 }, { image:Width( ) - 1, image:Height( ) - 1 }, 'FF0000' )

            -- Highlight motion areas
            if highlightMotion then
                addImages:ProcessImageInPlace( image, diff )
            end

            -- Update time of detected motion
            motionDetectionTime = os.clock( )
        end

        diff:Release( )
    else
        -- Initial version of background image
        backgroundImage = image:Clone( )
    end

    -- Check if we need to write current frame to a video file
    if os.clock( ) - motionDetectionTime <= extraSaveTime then
        if not videoFileStarted then
            -- Generate new file name
            videoWriter:SetProperty( 'fileName', os.date( '%Y-%m-%d %H-%M-%S' ) )

            videoFileStarted = true
        end

        -- Save current frame
        videoWriter:ProcessImage( image )
    else
        if videoFileStarted then
            -- Close video file
            videoWriter:Reset( )
            videoFileStarted = false
        end
    end
end
