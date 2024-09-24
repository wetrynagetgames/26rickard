/*
 * Copyright (c) 2023, Tim Flynn <trflynn89@serenityos.org>
 * Copyright (c) 2024, Sam Atkins <sam@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/String.h>
#include <AK/StringView.h>
#include <LibSyntax/Document.h>
#include <LibSyntax/HighlighterClient.h>
#include <LibSyntax/Language.h>

namespace WebView {

class SourceDocument final : public Syntax::Document {
public:
    static NonnullRefPtr<SourceDocument> create(StringView source)
    {
        return adopt_ref(*new (nothrow) SourceDocument(source));
    }
    virtual ~SourceDocument() = default;

    StringView text() const { return m_source; }
    size_t line_count() const { return m_lines.size(); }

    // ^ Syntax::Document
    virtual Syntax::TextDocumentLine const& line(size_t line_index) const override;
    virtual Syntax::TextDocumentLine& line(size_t line_index) override;

private:
    SourceDocument(StringView source);

    // ^ Syntax::Document
    virtual void update_views(Badge<Syntax::TextDocumentLine>) override { }

    StringView m_source;
    Vector<Syntax::TextDocumentLine> m_lines;
};

class SourceHighlighterClient final : public Syntax::HighlighterClient {
public:
    SourceHighlighterClient(StringView source, Syntax::Language);
    virtual ~SourceHighlighterClient() = default;

    String to_html_string(URL::URL const&) const;

private:
    // ^ Syntax::HighlighterClient
    virtual Vector<Syntax::TextDocumentSpan> const& spans() const override;
    virtual void set_span_at_index(size_t index, Syntax::TextDocumentSpan span) override;
    virtual Vector<Syntax::TextDocumentFoldingRegion>& folding_regions() override;
    virtual Vector<Syntax::TextDocumentFoldingRegion> const& folding_regions() const override;
    virtual ByteString highlighter_did_request_text() const override;
    virtual void highlighter_did_request_update() override;
    virtual Syntax::Document& highlighter_did_request_document() override;
    virtual Syntax::TextPosition highlighter_did_request_cursor() const override;
    virtual void highlighter_did_set_spans(Vector<Syntax::TextDocumentSpan>) override;
    virtual void highlighter_did_set_folding_regions(Vector<Syntax::TextDocumentFoldingRegion>) override;

    StringView class_for_token(u64 token_type) const;

    SourceDocument& document() const { return *m_document; }

    NonnullRefPtr<SourceDocument> m_document;
    OwnPtr<Syntax::Highlighter> m_highlighter;
};

String highlight_source(URL::URL const&, StringView);

constexpr inline StringView HTML_HIGHLIGHTER_STYLE = R"~~~(
    .html {
        font-size: 10pt;
        font-family: Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
    }

    .tag {
        font-weight: 600;
    }

    @media (prefers-color-scheme: dark) {
        /* FIXME: We should be able to remove the HTML style when "color-scheme" is supported */
        html {
            background-color: rgb(30, 30, 30);
            color: white;
        }
        .comment {
            color: lightgreen;
        }
        .tag {
            color: orangered;
        }
        .attribute-name {
            color: orange;
        }
        .attribute-value {
            color: deepskyblue;
        }
        .internal {
            color: darkgrey;
        }
    }

    @media (prefers-color-scheme: light) {
        .comment {
            color: green;
        }
        .tag {
            color: red;
        }
        .attribute-name {
            color: darkorange;
        }
        .attribute-value {
            color: blue;
        }
        .internal {
            color: dimgray;
        }
    }
)~~~"sv;

}
