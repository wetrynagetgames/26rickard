/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <LibGUI/Forward.h>
#include <LibGUI/Layout.h>
#include <LibGfx/Orientation.h>

namespace GUI {

class BoxLayout : public Layout {
    C_OBJECT(BoxLayout);

public:
    virtual ~BoxLayout() override { }

    Gfx::Orientation orientation() const { return m_orientation; }

    virtual void run(Widget&) override;
    virtual Gfx::IntSize preferred_size() const override;

protected:
    explicit BoxLayout(Gfx::Orientation);

private:
    int preferred_primary_size() const;
    int preferred_secondary_size() const;

    Gfx::Orientation m_orientation;
};

class VerticalBoxLayout final : public BoxLayout {
    C_OBJECT(VerticalBoxLayout);

public:
    explicit VerticalBoxLayout()
        : BoxLayout(Gfx::Orientation::Vertical)
    {
    }
    virtual ~VerticalBoxLayout() override { }
};

class HorizontalBoxLayout final : public BoxLayout {
    C_OBJECT(HorizontalBoxLayout);

public:
    explicit HorizontalBoxLayout()
        : BoxLayout(Gfx::Orientation::Horizontal)
    {
    }
    virtual ~HorizontalBoxLayout() override { }
};

}
