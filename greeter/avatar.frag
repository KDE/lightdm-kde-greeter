/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2023-2024 Anton Golubev <golubevan@altlinux.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#version 440

//draw a circle with an antialiased border
//innerRadius = size of the inner circle with contents
//outerRadius = size of the border
//blend = area to blend between two colours
//all sizes are normalised so 0.5 == half the width of the texture

layout(location = 0) in vec2 qt_TexCoord0;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 colorBorder;
};
layout(binding = 1) uniform sampler2D source;

layout(location = 0) out vec4 fragColor;

layout(constant_id = 1) const float blend = 0.01;
layout(constant_id = 2) const float innerRadius = 0.47;
layout(constant_id = 3) const float outerRadius = 0.49;
const vec4 colorEmpty = vec4(0.0, 0.0, 0.0, 0.0);

void main() {
    vec4 colorSource = texture(source, qt_TexCoord0.st);

    vec2 m = qt_TexCoord0 - vec2(0.5, 0.5);
    float dist = sqrt(m.x * m.x + m.y * m.y);

    if (dist < innerRadius)
        fragColor = colorSource;
    else if (dist < innerRadius + blend)
        fragColor = mix(colorSource, colorBorder, ((dist - innerRadius) / blend));
    else if (dist < outerRadius)
        fragColor = colorBorder;
    else if (dist < outerRadius + blend)
        fragColor = mix(colorBorder, colorEmpty, ((dist - outerRadius) / blend));
    else
        fragColor = colorEmpty;

    fragColor = fragColor * qt_Opacity;
}
