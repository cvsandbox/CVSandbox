--[[
    Computer Vision Sandbox, Lua scripting sample
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    The script demonstrates a video fading effect, which
    first randomly fills video frames with black blocks and
    then randomly cleans it back. Caps Lock key status must
    be ON to enable the effect. The effect starts on pressing
    Num Lock key.
--]]

local table = require "table"
local math  = require "math"

-- Make sure we have required APIs exposed
if SCRIPTING_API_REVISION == nil then
    error( 'The script requires minimum API revision 2' )
end

ledKeysPlugin = Host.CreatePluginInstance( 'LedKeys' )

-- Number of blocks in X/Y directions
xBlocks = 40
yBlocks = 30
-- Maximum number of block to fade per frame
maxRate = 10

running = false
fadeOut = true
blocks  = { }
map     = { }

-- Connect the LedKeys device plug-in
ledKeysPlugin:Connect( )
prevNumLockState = ledKeysPlugin:GetProperty( 'numLock' )

-- Main function to be executed for every frame
function Main( )
    -- Do nothing if Caps Lock is off
    if ( not ledKeysPlugin:GetProperty( 'capsLock' ) ) then
        running = false
        return
    end

    -- Start effect when NumLock status has changed
    if ( prevNumLockState ~= ledKeysPlugin:GetProperty( 'numLock' ) ) and ( not running ) then
        map     = GenerateMap( )
        blocks  = GenerateBlocksList( )
        running = true
    end

    if running then
        if #blocks == 0 then
            if fadeOut then
                -- Switch to fade in
                fadeOut = false
                map     = GenerateMap( )
                blocks  = GenerateBlocksList( )
            else
                -- Switch off the effect
                fadeOut = true
                running = false
            end
        else
            blocksToFade = maxRate - math.floor( ( #blocks * maxRate - 1 ) / ( xBlocks * yBlocks ) )

            -- Mark some blocks as faded
            for i = 1, blocksToFade do
                if #blocks > 0 then
                    pair = table.remove( blocks, math.random( #blocks ) )
                    map[ ( pair[2] - 1 ) * xBlocks + pair[1] ] = true
                end
            end
        end

        -- Apply the effect
        image = Host.GetImage( )

        width  = image:Width( )
        height = image:Height( )

        blockWidth  = width  / xBlocks
        blockHeight = height / yBlocks

        -- Create a small image, which is black by default
        blockImage = Image.Create( blockWidth, blockHeight, image:PixelFormat( ) )

        for x = 1, xBlocks do
            for y = 1, yBlocks do
                faded = map[ ( y - 1 ) * xBlocks + x ]

                if faded == fadeOut then
                    image:PutImage( blockImage, ( x - 1 ) * blockWidth, ( y - 1 ) * blockHeight )
                end
            end
        end

        blockImage:Release( )
    end

    prevNumLockState = ledKeysPlugin:GetProperty( 'numLock' )
end

-- Generate list of available blocks
function GenerateBlocksList( )
    blocks = { }

    for x = 1, xBlocks do
        for y = 1, yBlocks do
            blocks[#blocks + 1] = { x, y }
        end
    end

    return blocks
end

-- Generate map indicating which blocks are faded
function GenerateMap( )
    map  = { }
    size = xBlocks * yBlocks

    for i = 1, size do
        map[#map + 1] = false
    end

    return map
end
