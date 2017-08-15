/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    float4 pixelColor;
    /*
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    */
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;


    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    output.position = input.position;
    output.position.w = 2.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    //output.position = mul(input.position, worldMatrix);
    //output.position = mul(output.position, viewMatrix);
    //output.position = mul(output.position, projectionMatrix);

    // Store the input color for the pixel shader to use.
    output.color = input.color * pixelColor;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
    return input.color;
}