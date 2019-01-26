--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    A sample script which uses Pixellate plug-in to "change"
    vertical/horizontal size of pixels. It first increases
    size of pixels and then decreases, repeating it a loop.
--]]

-- Create instance of plug-in to use
pixellatePlugin = Host.CreatePluginInstance( 'Pixellate' )

-- Start values of some properties
pixelWidth  = 1
pixelHeight = 1

-- Other variables
counter      = 0
increasing   = true
maxPixelSize = 16

-- Main function to be executed for every frame
function Main( )
    -- Get image to process
    image = Host.GetImage( )
    -- Apply image processing routine
    pixellatePlugin:ProcessImageInPlace( image )

    -- Update properties
    if counter == 0 then
        if increasing then
            -- increase pixel width first, then height
            if pixelWidth == maxPixelSize then
                pixelHeight = pixelHeight + 1
                if pixelHeight == maxPixelSize then
                    increasing = false
                end
            else
                pixelWidth = pixelWidth + 1
            end
        else
            -- decrease pixel width first, then height
            if pixelWidth == 1 then
                pixelHeight = pixelHeight - 1
                if pixelHeight == 1 then
                    increasing = true
                end
            else
                pixelWidth = pixelWidth - 1
            end
        end

        pixellatePlugin:SetProperty( 'pixelWidth', pixelWidth )
        pixellatePlugin:SetProperty( 'pixelHeight', pixelHeight )
    end

    counter = ( counter + 1 ) % 5
end
