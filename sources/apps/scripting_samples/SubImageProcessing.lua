--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    The script demonstrates how to apply image processing
    plug-ins to sub images of the source image.
--]]

-- Create instances of plug-ins to use
setHuePlugin = Host.CreatePluginInstance( 'SetHue' )
sepiaPlugin  = Host.CreatePluginInstance( 'Sepia' )

grayscalePlugin      = Host.CreatePluginInstance( 'Grayscale' )
grayscaleToRgbPlugin = Host.CreatePluginInstance( 'GrayscaleToRgb' )

pixellatePlugin = Host.CreatePluginInstance( 'Pixellate' )

-- Counter used to switch between effects
counter = 0

-- Main function to be executed for every frame
function Main( )
    -- Make sure we have required APIs exposed
    if SCRIPTING_API_REVISION == nil then
        error( 'The script requires minimum API revision 2' )
    end

    -- Get image to process
    image = Host.GetImage( )

    width  = image:Width( )
    height = image:Height( )

    halfWidth  = width  / 2;
    halfHeight = height / 2;

    -- Extract center sub image for later processing
    centerImage = image:GetSubImage( halfWidth / 2, halfHeight / 2, halfWidth, halfHeight ):Clone( )

    if counter < 150 then
        -- process left part of the image
        setHuePlugin:SetProperty( 'hue', 0 )
        setHuePlugin:ProcessImageInPlace( image:GetSubImage( 0, 0, halfWidth, height ) )
        -- process right part of the image
        setHuePlugin:SetProperty( 'hue', 45 )
        setHuePlugin:ProcessImageInPlace( image:GetSubImage( width - halfWidth, 0, halfWidth, height ) )
        -- process center and put it back
        sepiaPlugin:ProcessImageInPlace( centerImage )
        image:PutImage( centerImage, halfWidth / 2, halfHeight / 2 )

    elseif ( counter >= 150 ) and ( counter < 300 ) then
        -- process top part of the image
        grayImage = grayscalePlugin:ProcessImage( image:GetSubImage( 0, 0, width, halfHeight ) )
        -- grayscale plug-in provides new image of different format,
        -- so it is required to convert it back 24 bpp and put it back
        grayRgbImage = grayscaleToRgbPlugin:ProcessImage( grayImage )
        image:PutImage( grayRgbImage, 0, 0 )

        -- process bottom part of the image
        setHuePlugin:SetProperty( 'hue', 135 )
        setHuePlugin:ProcessImageInPlace( image:GetSubImage( 0, height - halfHeight, width, halfHeight ) )
        -- process center and put it back
        setHuePlugin:SetProperty( 'hue', 225 )
        setHuePlugin:ProcessImageInPlace( centerImage )
        image:PutImage( centerImage, halfWidth / 2, halfHeight / 2 )

        grayImage:Release( )
        grayRgbImage:Release( )

    else
        -- apply pixellate to the entire image
        pixellatePlugin:ProcessImageInPlace( image )
        -- put center image back as is without any processing
        image:PutImage( centerImage, halfWidth / 2, halfHeight / 2 )
    end

    counter = ( counter + 1 ) % 450

    -- Release the cloned sub image
    centerImage:Release( )
end
