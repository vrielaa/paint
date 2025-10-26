#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cstdint>

#include <algorithm>

struct Line {
    sf::Vector2f start;
    sf::Vector2f end;
    sf::Color firstColor;
    sf::Color secondColor;
};

enum ToolType {
    TOOL_LINE,
    TOOL_RECTANGLE,
    TOOL_FILLED_RECTANGLE,
    TOOL_CIRCLE
};

enum LineMode {
    LINE_MODE_ONE_COLOR,
    LINE_MODE_GRADIENT
};

class PaintApp {
public:
    int selectedTool = TOOL_LINE;
    int selectedLineMode = LINE_MODE_ONE_COLOR;
    bool isToolsShown = true;
    bool isDrawingLine = false;
    bool isLineGradient = false;
    bool isDrawingRectangle = false;
    bool isRectangleFilled = false;
    bool isDrawingCircle = false;
    float brushSize = 12.0f;
    std::vector<Line> lines;
    std::vector<sf::RectangleShape> rectangles;
    sf::Vector2f lineStart{}, lineEnd{};
    sf::Vector2f rectangleStart{}, rectangleEnd{};
    sf::RectangleShape tempRectangle;
    std::vector<sf::CircleShape> circles;
    sf::Vector2f circleStart{}, circleEnd{};
    sf::CircleShape tempCircle;
    sf::Color currentBorderColor = sf::Color::Black;
    sf::Color currentFillColor = sf::Color::Black;

    PaintApp() {
    }

    void run() {
    }

    void menuBar(sf::RenderWindow& window);
    void newFile(sf::RenderWindow& window);
    void openFile(const std::string& filename);
    void saveFileAs(const std::string& filename);
    void exit();

    void help();
    void drawToolsWindow(sf::RenderWindow& window);
    void keepImGuiWindowInside(const sf::RenderWindow& sfWindow, float margin = 0.0f);

    void lineTool(sf::RenderWindow& window);
    void rectangleTool(sf::RenderWindow& window, bool filled);
    void circleTool(sf::RenderWindow& window);

    void chosenTool(sf::RenderWindow& window);
    void renderCanvas(sf::RenderWindow& window);
};

void PaintApp::menuBar(sf::RenderWindow& window) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N", false, true)) {
                newFile(window);
            }
            if (ImGui::MenuItem("Open...", "Ctrl+O", false, true)) {
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+S", false, true)) {
            }
            if (ImGui::MenuItem("Exit", "Alt+F4", false, true)) {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Show Tool Options", "", isToolsShown)) {
                isToolsShown = !isToolsShown;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About", "", false, true)) {
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void PaintApp::keepImGuiWindowInside(const sf::RenderWindow& sfWindow, float margin) {
    ImVec2 toolPosition = ImGui::GetWindowPos();
    ImVec2 toolSize = ImGui::GetWindowSize();
    sf::Vector2u windowSizeSFML = sfWindow.getSize();

    float targetToolWidth = std::min(toolSize.x, static_cast<float>(windowSizeSFML.x));
    float targetToolHeight = std::min(toolSize.y, static_cast<float>(windowSizeSFML.y));

    if (toolSize.x != targetToolWidth) {
        ImGui::SetWindowSize(ImVec2(targetToolWidth - margin, toolSize.y));
    }

    if (toolSize.y != targetToolHeight) {
        ImGui::SetWindowSize(ImVec2(toolSize.x, targetToolHeight - margin));
    }

    toolSize = ImGui::GetWindowSize();
    toolPosition = ImGui::GetWindowPos();

    float maxX = std::max(0.0f, static_cast<float>(windowSizeSFML.x) - toolSize.x);
    float maxY = std::max(0.0f, static_cast<float>(windowSizeSFML.y) - toolSize.y);
    ImVec2 clampedPos = ImVec2(
        std::clamp(toolPosition.x, 0.0f, maxX),
        std::clamp(toolPosition.y, 0.0f, maxY)
    );

    if (clampedPos.x != toolPosition.x || clampedPos.y != toolPosition.y) {
        ImGui::SetWindowPos(clampedPos);
    }
}

void PaintApp::drawToolsWindow(sf::RenderWindow& window) {
    if (!isToolsShown) return;

    ImGui::SetNextWindowPos(ImVec2(20, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(260, 200), ImGuiCond_FirstUseEver);
    float margin = 25.0f;

    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.839f, 0.941f, 0.867f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.447f, 0.792f, 0.533f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.1f, 0.25f, 0.1f, 1.0f));

    if (ImGui::Begin("Tools", &isToolsShown)) {
        keepImGuiWindowInside(window, margin);

        ImGui::TextUnformatted("Tool Options");
        ImGui::Separator();

        ImGui::RadioButton("Line", &selectedTool, TOOL_LINE);
        ImGui::RadioButton("Rectangle", &selectedTool, TOOL_RECTANGLE);
        ImGui::RadioButton("Filled Rectangle", &selectedTool, TOOL_FILLED_RECTANGLE);
        ImGui::RadioButton("Circle", &selectedTool, TOOL_CIRCLE);
        ImGui::SliderFloat("Brush Size", &brushSize, 1.0f, 100.0f);

        ImGui::Separator();

        if (selectedTool == TOOL_LINE) {
            ImGui::TextUnformatted("Line Options:");
            ImGui::RadioButton("One color", &selectedLineMode, LINE_MODE_ONE_COLOR);
            ImGui::RadioButton("Gradient", &selectedLineMode, LINE_MODE_GRADIENT);
        }

        static float color1[3] = { 1.f, 0.2f, 0.3f };
        ImGui::ColorEdit3("Border Color", color1);
        currentBorderColor = sf::Color(
            static_cast<uint8_t>(color1[0] * 255.f),
            static_cast<uint8_t>(color1[1] * 255.f),
            static_cast<uint8_t>(color1[2] * 255.f)
        );

        static float color2[3] = { 1.f, 0.2f, 0.3f };
        ImGui::ColorEdit3("Fill Color", color2);
        currentFillColor = sf::Color(
            static_cast<uint8_t>(color2[0] * 255.f),
            static_cast<uint8_t>(color2[1] * 255.f),
            static_cast<uint8_t>(color2[2] * 255.f)
        );
    }
    ImGui::End();

    ImGui::PopStyleColor(3);
}

void PaintApp::chosenTool(sf::RenderWindow& window) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    switch (selectedTool) {
    case TOOL_LINE:
        lineTool(window);
        break;
    case TOOL_RECTANGLE:
        rectangleTool(window, false);
        break;
    case TOOL_FILLED_RECTANGLE:
        rectangleTool(window, true);
        break;
    case TOOL_CIRCLE:
        circleTool(window);
        break;
    default:
        break;
    }
}

void PaintApp::lineTool(sf::RenderWindow& window) {
    if (selectedTool != TOOL_LINE) return;

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    static bool previousMouseState = false;
    bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    if (!isDrawingLine) {
        if (pressed && !previousMouseState) {
            isDrawingLine = true;
            lineStart = lineEnd = mouse;
        }
    }
    else {
        lineEnd = mouse;

        if (!pressed && previousMouseState) {
            if (selectedLineMode == LINE_MODE_ONE_COLOR) {
                isLineGradient = false;
                Line L;
                L.start = lineStart;
                L.end = lineEnd;
                L.firstColor = L.secondColor = currentBorderColor;
                lines.push_back(L);
            }
            else if (selectedLineMode == LINE_MODE_GRADIENT) {
                isLineGradient = true;
                Line L;
                L.start = lineStart;
                L.end = lineEnd;
                L.firstColor = currentBorderColor;
                L.secondColor = currentFillColor;
                lines.push_back(L);
            }
            isDrawingLine = false;
        }
    }

    previousMouseState = pressed;
}

void PaintApp::rectangleTool(sf::RenderWindow& window, bool filled) {
    if (selectedTool != TOOL_RECTANGLE && selectedTool != TOOL_FILLED_RECTANGLE) return;

    if (filled) isRectangleFilled = true;
    else isRectangleFilled = false;

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    static bool previousMouseState = false;
    bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    if (!isDrawingRectangle) {
        if (pressed && !previousMouseState) {
            isDrawingRectangle = true;
            rectangleStart = rectangleEnd = mouse;
        }
    }
    else {
        rectangleEnd = mouse;

        if (!pressed && previousMouseState) {
            sf::RectangleShape rect;
            rect.setPosition(sf::Vector2f(
                std::min(rectangleStart.x, rectangleEnd.x),
                std::min(rectangleStart.y, rectangleEnd.y)
            ));
            rect.setSize(sf::Vector2f(
                std::abs(rectangleEnd.x - rectangleStart.x),
                std::abs(rectangleEnd.y - rectangleStart.y)
            ));
            if (filled)
                rect.setFillColor(currentFillColor);
            else
                rect.setFillColor(sf::Color::Transparent);
            rect.setOutlineColor(currentBorderColor);
            rect.setOutlineThickness(brushSize);
            rectangles.push_back(rect);
            isDrawingRectangle = false;
        }
    }

    previousMouseState = pressed;
}

void PaintApp::circleTool(sf::RenderWindow& window) {
    if (selectedTool != TOOL_CIRCLE) return;

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    static bool previousMouseState = false;
    bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    if (!isDrawingCircle) {
        if (pressed && !previousMouseState) {
            isDrawingCircle = true;
            circleStart = circleEnd = mouse;
        }
    }
    else {
        circleEnd = mouse;

        if (!pressed && previousMouseState) {
            float radius = std::sqrt(
                std::pow(circleEnd.x - circleStart.x, 2) +
                std::pow(circleEnd.y - circleStart.y, 2)
            );
            sf::CircleShape circle(radius);
            circle.setPosition(sf::Vector2f(
                circleStart.x - radius,
                circleStart.y - radius
            ));
            circle.setFillColor(sf::Color::Transparent);
            circle.setOutlineColor(currentBorderColor);
            circle.setOutlineThickness(brushSize);
            circles.push_back(circle);
            isDrawingCircle = false;
        }
    }

    previousMouseState = pressed;
}

void PaintApp::renderCanvas(sf::RenderWindow& window) {
    for (const auto& line : lines) {
        sf::Vertex v[2];
        v[0].position = line.start;
        v[0].color = line.firstColor;
        v[1].position = line.end;
        v[1].color = line.secondColor;
        window.draw(v, 2, sf::PrimitiveType::Lines);
    }

    for (const auto& rect : rectangles) {
        window.draw(rect);
    }

    for (const auto& circle : circles) {
        window.draw(circle);
    }

    if (isDrawingLine) {
        sf::Vertex v[2];
        if (isLineGradient) {
            v[0].color = currentBorderColor;
            v[1].color = currentFillColor;
        }
        else {
            v[0].color = currentBorderColor;
            v[1].color = currentBorderColor;
        }
        v[0].position = lineStart;
        v[1].position = lineEnd;
        window.draw(v, 2, sf::PrimitiveType::Lines);
    }

    if (isDrawingRectangle) {
        float width = std::abs(rectangleEnd.x - rectangleStart.x);
        float height = std::abs(rectangleEnd.y - rectangleStart.y);
        tempRectangle.setPosition(sf::Vector2f(
            std::min(rectangleStart.x, rectangleEnd.x),
            std::min(rectangleStart.y, rectangleEnd.y)
        ));
        tempRectangle.setSize(sf::Vector2f(width, height));

        if (isRectangleFilled)
            tempRectangle.setFillColor(currentFillColor);
        else
            tempRectangle.setFillColor(sf::Color::Transparent);

        tempRectangle.setOutlineColor(currentBorderColor);
        tempRectangle.setOutlineThickness(brushSize);

        window.draw(tempRectangle);
    }

    if (isDrawingCircle) {
        float radius = std::sqrt(
            std::pow(circleEnd.x - circleStart.x, 2) +
            std::pow(circleEnd.y - circleStart.y, 2)
        );
        tempCircle.setRadius(radius);
        tempCircle.setPosition(sf::Vector2f(
            circleStart.x - radius,
            circleStart.y - radius
        ));
        tempCircle.setFillColor(sf::Color::Transparent);
        tempCircle.setOutlineColor(currentBorderColor);
        tempCircle.setOutlineThickness(brushSize);
        window.draw(tempCircle);
    }
}

void PaintApp::newFile(sf::RenderWindow& window) {
    window.clear(sf::Color::White);
}

int main() {
    sf::RenderWindow window(sf::VideoMode({ 1800, 900 }), "ImGui + SFML");
    sf::Clock deltaClock;
    PaintApp app;

    window.setFramerateLimit(60);
    window.clear(sf::Color::White);

    std::ignore = ImGui::SFML::Init(window);
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        app.chosenTool(window);

        app.menuBar(window);
        app.drawToolsWindow(window);
        window.clear(sf::Color::White);
        app.renderCanvas(window);
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
}
