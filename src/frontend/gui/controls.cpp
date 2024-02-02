#include "controls.h"

#include <iostream>

#include "../../window/Events.h"
#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/Font.h"
#include "../../graphics/GfxContext.h"
#include "../../util/stringutil.h"
#include "GUI.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;

using namespace gui;

Label::Label(std::string text, std::string fontName) 
     : UINode(vec2(), vec2(text.length() * 8, 15)), 
       text(util::str2wstr_utf8(text)), 
       fontName_(fontName) {
    setInteractive(false);
}


Label::Label(std::wstring text, std::string fontName) 
     : UINode(vec2(), vec2(text.length() * 8, 15)), 
       text(text), 
       fontName_(fontName) {
    setInteractive(false);
}

void Label::setText(std::wstring text) {
    this->text = text;
}

std::wstring Label::getText() const {
    return text;
}

void Label::draw(const GfxContext* pctx, Assets* assets) {
    if (supplier) {
        setText(supplier());
    }

    auto batch = pctx->getBatch2D();
    batch->color = getColor();
    Font* font = assets->getFont(fontName_);
    vec2 size = getSize();
    vec2 newsize = vec2(font->calcWidth(text), font->lineHeight());
    if (newsize.x > size.x) {
        setSize(newsize);
        size = newsize;
    }
    vec2 coord = calcCoord();
    font->draw(batch, text, coord.x, coord.y);
}

Label* Label::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
    return this;
}

void Label::setSize(vec2 sizenew) {
    UINode::setSize(vec2(UINode::getSize().x, sizenew.y));
}

// ================================= Image ====================================
Image::Image(std::string texture, vec2 size) : UINode(vec2(), size), texture(texture) {
    setInteractive(false);
}

void Image::draw(const GfxContext* pctx, Assets* assets) {
    vec2 coord = calcCoord();
    vec4 color = getColor();
    auto batch = pctx->getBatch2D();
    batch->texture(assets->getTexture(texture));
    batch->color = color;
    batch->rect(coord.x, coord.y, size.x, size.y, 
                0, 0, 0, UVRegion(), false, true, color);
}

// ================================= Button ===================================
Button::Button(std::shared_ptr<UINode> content, glm::vec4 padding)
    : Panel(vec2(), padding, 0) {
    vec4 margin = getMargin();
    setSize(content->getSize()+vec2(padding[0]+padding[2]+margin[0]+margin[2],
                                    padding[1]+padding[3]+margin[1]+margin[3]));
    add(content);
    scrollable(false);
}

Button::Button(std::wstring text, glm::vec4 padding, onaction action) 
 : Panel(vec2(32,32), padding, 0) 
{
    if (action) {
        listenAction(action);
    }
    scrollable(false);

    label = std::make_shared<Label>(text);
    label->setAlign(Align::center);
    add(label);
}

void Button::setText(std::wstring text) {
    if (label) {
        label->setText(text);
    }
}

std::wstring Button::getText() const {
    if (label) {
        return label->getText();
    }
    return L"";
}

Button* Button::textSupplier(wstringsupplier supplier) {
    if (label) {
        label->textSupplier(supplier);
    }
    return this;
}

void Button::setHoverColor(glm::vec4 color) {
    hoverColor = color;
}

void Button::drawBackground(const GfxContext* pctx, Assets* assets) {
    vec2 coord = calcCoord();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->color = (isPressed() ? pressedColor : (hover ? hoverColor : color));
    batch->rect(coord.x, coord.y, size.x, size.y);
}

std::shared_ptr<UINode> Button::getAt(vec2 pos, std::shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void Button::mouseRelease(GUI* gui, int x, int y) {
    UINode::mouseRelease(gui, x, y);
    if (isInside(vec2(x, y))) {
        for (auto callback : actions) {
            callback(gui);
        }
    }
}

Button* Button::listenAction(onaction action) {
    actions.push_back(action);
    return this;
}

void Button::textAlign(Align align) {
    if (label) {
        label->setAlign(align);
        refresh();
    }
}

// ============================== RichButton ==================================
RichButton::RichButton(vec2 size) : Container(vec2(), size) {
}

void RichButton::mouseRelease(GUI* gui, int x, int y) {
    UINode::mouseRelease(gui, x, y);
    if (isInside(vec2(x, y))) {
        for (auto callback : actions) {
            callback(gui);
        }
    }
}

RichButton* RichButton::listenAction(onaction action) {
    actions.push_back(action);
    return this;
}

void RichButton::setHoverColor(glm::vec4 color) {
    hoverColor = color;
}

void RichButton::drawBackground(const GfxContext* pctx, Assets* assets) {
    vec2 coord = calcCoord();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->color = (isPressed() ? pressedColor : (hover ? hoverColor : color));
    batch->rect(coord.x, coord.y, size.x, size.y);
}

// ================================ TextBox ===================================
TextBox::TextBox(std::wstring placeholder, vec4 padding) 
    : Panel(vec2(200,32), padding, 0, false), 
      input(L""),
      placeholder(placeholder) {
    label = std::make_shared<Label>(L"");
    add(label);
}

void TextBox::drawBackground(const GfxContext* pctx, Assets* assets) {
    vec2 coord = calcCoord();

    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);

    if (valid) {
        if (isFocused()) {
            batch->color = focusedColor;
        } else if (hover) {
            batch->color = hoverColor;
        } else {
            batch->color = color;
        }
    } else {
        batch->color = invalidColor;
    }

    batch->rect(coord.x, coord.y, size.x, size.y);
    if (!isFocused() && supplier) {
        input = supplier();
    }

    if (input.empty()) {
        label->setColor(vec4(0.5f));
        label->setText(placeholder);
    } else {
        label->setColor(vec4(1.0f));
        label->setText(input);
    }
    scrollable(false);
}

void TextBox::typed(unsigned int codepoint) {
    input += std::wstring({(wchar_t)codepoint});
    validate();
}

bool TextBox::validate() {
    if (validator) {
        valid = validator(input);
    } else {
        valid = true;
    }
    return valid;
}

void TextBox::setValid(bool valid) {
    this->valid = valid;
}

bool TextBox::isValid() const {
    return valid;
}

void TextBox::setOnEditStart(gui::runnable oneditstart) {
    onEditStart = oneditstart;
}

void TextBox::focus(GUI* gui) {
    Panel::focus(gui);
    if (onEditStart){
        onEditStart();
    }
}

void TextBox::keyPressed(int key) {
    if (key == keycode::BACKSPACE) {
        if (!input.empty()){
            input = input.substr(0, input.length()-1);
            validate();
        }
    } else if (key == keycode::ENTER) {
        if (validate() && consumer) {
            consumer(label->getText());
        }
        defocus();        
    }
    // Pasting text from clipboard
    if (key == keycode::V && Events::pressed(keycode::LEFT_CONTROL)) {
        const char* text = Window::getClipboardText();
        if (text) {
            input += util::str2wstr_utf8(text);
            validate();
        }
    }
}

std::shared_ptr<UINode> TextBox::getAt(vec2 pos, std::shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void TextBox::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

void TextBox::textConsumer(wstringconsumer consumer) {
    this->consumer = consumer;
}

void TextBox::textValidator(wstringchecker validator) {
    this->validator = validator;
}

std::wstring TextBox::text() const {
    if (input.empty())
        return placeholder;
    return input;
}

void TextBox::text(std::wstring value) {
    this->input = value;
}

// ============================== InputBindBox ================================
InputBindBox::InputBindBox(Binding& binding, vec4 padding) 
    : Panel(vec2(100,32), padding, 0, false),
      binding(binding) {
    label = std::make_shared<Label>(L"");
    add(label);
    scrollable(false);
}

void InputBindBox::drawBackground(const GfxContext* pctx, Assets* assets) {
    vec2 coord = calcCoord();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->color = (isFocused() ? focusedColor : (hover ? hoverColor : color));
    batch->rect(coord.x, coord.y, size.x, size.y);
    label->setText(util::str2wstr_utf8(binding.text()));
}

void InputBindBox::clicked(GUI*, int button) {
    binding.type = inputtype::mouse;
    binding.code = button;
    defocus();
}

void InputBindBox::keyPressed(int key) {
    if (key != keycode::ESCAPE) {
        binding.type = inputtype::keyboard;
        binding.code = key;
    }
    defocus();
}

// ================================ TrackBar ==================================
TrackBar::TrackBar(double min, 
                   double max, 
                   double value, 
                   double step, 
                   int trackWidth)
    : UINode(vec2(), vec2(26)), 
      min(min), 
      max(max), 
      value(value), 
      step(step), 
      trackWidth(trackWidth) {
    setColor(vec4(0.f, 0.f, 0.f, 0.4f));
}

void TrackBar::draw(const GfxContext* pctx, Assets* assets) {
    if (supplier_) {
        value = supplier_();
    }
    vec2 coord = calcCoord();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->color = (hover ? hoverColor : color);
    batch->rect(coord.x, coord.y, size.x, size.y);

    float width = size.x;
    float t = (value - min) / (max-min+trackWidth*step);

    batch->color = trackColor;
    int actualWidth = size.x * (trackWidth / (max-min+trackWidth*step) * step);
    batch->rect(coord.x + width * t, coord.y, actualWidth, size.y);
}

void TrackBar::supplier(doublesupplier supplier) {
    this->supplier_ = supplier;
}

void TrackBar::consumer(doubleconsumer consumer) {
    this->consumer_ = consumer;
}

void TrackBar::mouseMove(GUI*, int x, int y) {
    vec2 coord = calcCoord();
    value = x;
    value -= coord.x;
    value = (value)/size.x * (max-min+trackWidth*step);
    value += min;
    value = (value > max) ? max : value;
    value = (value < min) ? min : value;
    value = (int)(value / step) * step;
    if (consumer_) {
        consumer_(value);
    }
}

// ================================ CheckBox ==================================
CheckBox::CheckBox(bool checked) : UINode(vec2(), vec2(32.0f)), checked_(checked) {
    setColor(vec4(0.0f, 0.0f, 0.0f, 0.5f));
}

void CheckBox::draw(const GfxContext* pctx, Assets* assets) {
    if (supplier_) {
        checked_ = supplier_();
    }
    vec2 coord = calcCoord();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->color = checked_ ? checkColor : (hover ? hoverColor : color);
    batch->rect(coord.x, coord.y, size.x, size.y);
}

void CheckBox::mouseRelease(GUI*, int x, int y) {
    checked_ = !checked_;
    if (consumer_) {
        consumer_(checked_);
    }
}

void CheckBox::supplier(boolsupplier supplier) {
    supplier_ = supplier;
}

void CheckBox::consumer(boolconsumer consumer) {
    consumer_ = consumer;
}

CheckBox* CheckBox::checked(bool flag) {
    checked_ = flag;
    return this;
}

FullCheckBox::FullCheckBox(std::wstring text, glm::vec2 size, bool checked)
    : Panel(size), 
      checkbox(std::make_shared<CheckBox>(checked)){
    setColor(vec4(0.0f));
    orientation(Orientation::horizontal);

    add(checkbox);

    auto label = std::make_shared<Label>(text); 
    label->setMargin(vec4(5.f, 5.f, 0.f, 0.f));
    add(label);
}
