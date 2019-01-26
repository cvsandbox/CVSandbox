--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    The script demonstrates implementation of a simple
    motion detector based on difference between two
    consecutive frames of a video stream.
--]]

-- Create instances of plug-ins to use
diffImages   = Host.CreatePluginInstance( 'DiffImagesThresholded' )
addImages    = Host.CreatePluginInstance( 'AddImages' )
imageDrawing = Host.CreatePluginInstance( 'ImageDrawing' )

-- Since we deal with RGB images, set threshold to 60 for the sum
-- of RGB differences
diffImages:SetProperty( 'threshold', 60 )
-- Highlight motion area with red color
diffImages:SetProperty( 'hiColor', 'FF0000' )

-- Amount of difference image to add to the source image for motion highlighting
addImages:SetProperty( 'factor', 0.3 )

-- Motion alarm threshold
motionThreshold = 0.1

-- Highlight motion areas or not
highlightMotion = true

function Main( )
    image = Host.GetImage( )

    if oldImage ~= nil then
        -- Calculate difference between current and the previous frames
        diff = diffImages:ProcessImage( image, oldImage )

        -- Set previous frame to the current one
        oldImage:Release( )
        oldImage = image:Clone( )

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
        end

        diff:Release( )
    else
        oldImage = image:Clone( )
    end
end
