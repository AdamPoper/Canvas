#include <AP2DGL/Renderer.h>
#include <AP2DGL/API_Tools/Clock.h>
#include <AP2DGL/API_Tools/FileDialog.h>

#include "Drawable.h"

#include <memory>
#include <vector>
#include <array>
#include <future>
#include <Windows.h>
#include <commdlg.h>
#include <variant>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define WIDTH 1440.0f
#define HEIGHT 900.0f

ap::Vec4f convertVector4(const ImVec4& vec)
{
	return { vec.x, vec.y, vec.z, vec.w };
}
ap::Vec3f convertVector3(const ImVec4& vec)
{
	return { vec.x, vec.y, vec.z };
}
ap::Vec2f convertVector2(float* f)
{
	return { f[0], f[1] };
}

void ShowMainMenuBar(ap::Window* win, ap::Renderer& renderer, SaveScreenShotData& saveArea, std::vector<Drawable*>& drawables)
{
	std::string filepath;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				filepath = ap::FileDialog::SaveFile(ap::FileDialog::DefaultFileFilter, win);
				if (!saveArea.areaDefined)
					renderer.ScreenShot(filepath.c_str());
				else if (saveArea.areaDefined)
				{
					renderer.ScreenShot(filepath.c_str(), saveArea.getStart(), saveArea.getSize());
					saveArea.reset();
				}
				
			}
			if (ImGui::MenuItem("Save as"))
			{				
			}
			if (ImGui::MenuItem("Open..."))  
			{
				filepath = ap::FileDialog::OpenFile("Text Files\0*.txt\0Any File\0*.*\0", win);
				if (!filepath.empty())
					std::cout << filepath << std::endl;
			}
			if (ImGui::MenuItem("New"))     {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z"))
			{
				delete drawables.back();
				drawables.pop_back();
				std::cout << "Size: " << drawables.size() << std::endl;
			}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Edit Screen Save Area"))
				saveArea.started = true;							
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
void defineSaveCanvasArea(SaveScreenShotData& saveAreaData, ap::Window& window)
{
	if (ap::Window::isMouseButtonPressed(ap::AP_MOUSE_BUTTON_LEFT) && saveAreaData.started)
	{
		saveAreaData.setStartPosition(window.getMousePosition());
		saveAreaData.definingArea = true;
		saveAreaData.started = false;
	}
	else if (saveAreaData.definingArea)
	{
		if (!saveAreaData.areaDefined)
			saveAreaData.update(window.getMousePosition());
		if (ap::Window::isMouseButtonPressed(ap::AP_MOUSE_BUTTON_RIGHT))
		{
			saveAreaData.areaDefined = true;
			saveAreaData.definingArea = false;
		}
	}
}
void updateEraseVerticies(std::vector<Drawable*>& drawables, ImVec4 clearcolor)
{
	for (int i = 0; i < drawables.size(); i++)
	{
		if (drawables[i]->type == Type::LINE)
		{
			Line* l = dynamic_cast<Line*>(drawables[i]);
			if (l != nullptr && l->buffer.Type() == DrawType::ERASE)
					l->buffer.SetColor(convertVector4(clearcolor));
		}
	}
}
void renderCustomShape(const CustomShape& cs, ap::Renderer& renderer)
{
	renderer.Draw(cs.vertices, cs.vertexCount, ap::PRIMITIVES::POLYGON);
}
void SubmitDrawableRenderData(Drawable* drawable, ap::Renderer& renderer)
{
	switch (drawable->type)
	{
		case Type::LINE:
		{
			Line* line = dynamic_cast<Line*>(drawable);
			if (line != nullptr)
			{
				if (line->buffer.Type() == DrawType::DRAW)
					line->buffer.Draw(renderer);
				else if (line->buffer.Type() == DrawType::ERASE)
					line->buffer.Draw(renderer);
			}
			return;
		}
		case Type::QUAD:
		{
			Quad* q = dynamic_cast<Quad*>(drawable);
			if (q != nullptr)
				renderer.DrawQuad(q->quad.getPosition(), q->quad.getSize(), q->quad.getColor(), q->quad.getRotation());
			return;
		}
		case Type::CIRCLE:
		{
			Circle* c = dynamic_cast<Circle*>(drawable);
			if (c != nullptr)
				renderer.DrawCircle(c->circle.getPosition(), c->circle.GetRadius(), c->circle.getColor());
			return;
		}
		case Type::TRIANGLE:
		{
			Triangle* t = dynamic_cast<Triangle*>(drawable);
			if (t != nullptr)
				renderer.DrawTriangle(t->triangle.getPosition(), t->triangle.getSize(), t->triangle.getColor());
			return;
		}
		case Type::IMAGE:
		{
			Image* img = dynamic_cast<Image*>(drawable);			
			if (img != nullptr)
				renderer.DrawQuad(img->image.getPosition(), img->image.getSize(), &img->texture, img->image.getRotation());									
			return;
		}
		case Type::CUSTOM:
		{
			CustomShape* cs = dynamic_cast<CustomShape*>(drawable);
			if(cs != nullptr)
				renderCustomShape(*cs, renderer);
			return;
		}
	}		
}
Quad* makeQuad(Drawable* prev)
{
	Quad* newQuad = new Quad;
	newQuad->type = Type::QUAD;
	Quad* q = dynamic_cast<Quad*>(prev);
	newQuad->quad.setColor(q->quad.getColor());
	newQuad->quad.setPosition(q->quad.getPosition());
	newQuad->quad.setSize(q->quad.getSize());
	newQuad->quad.rotate(q->quad.getRotation());
	return newQuad;
}
Circle* makeCircle(Drawable* prev)
{
	Circle* newCircle = new Circle;
	newCircle->type = Type::CIRCLE;
	Circle* c = dynamic_cast<Circle*>(prev);
	newCircle->circle.setColor(c->circle.getColor());
	newCircle->circle.setPosition(c->circle.getPosition());
	newCircle->circle.setRadius(c->circle.GetRadius());
	return newCircle;
}
Triangle* makeTriangle(Drawable* prev)
{
	Triangle* newTri = new Triangle;
	newTri->type = Type::TRIANGLE;
	Triangle* t = dynamic_cast<Triangle*>(prev);
	newTri->triangle.setColor(t->triangle.getColor());
	newTri->triangle.setPosition(t->triangle.getPosition());
	newTri->triangle.setSize(t->triangle.getSize());
	return newTri;
}
Image* makeImage(Drawable* prev)
{	
	Image* i = dynamic_cast<Image*>(prev);
	Image* img = new Image(i->filepath.c_str());
	img->type = Type::IMAGE;
	img->image.setPosition(i->image.getPosition());
	img->image.setSize(i->image.getSize());
	return img;
}
void QuadUI(PreviewConfiguration& qc, float windowWidth)
{
	ImGui::Checkbox("Init Quad Preview", &qc.showPreview);
	ImGui::ColorEdit4("Quad Color", (float*)&qc.previewColor);
	ImGui::SliderFloat2("Quad Position", qc.previewPosition, 0.0f, windowWidth);
	ImGui::DragFloat2("Quad Position", qc.previewPosition, 1.0f, -20.0f, 20.0f);
	ImGui::SliderFloat2("Quad Size", qc.previewSize, 0.0f, 1000.0f);
	ImGui::SliderFloat("Quad Rotation", &qc.previewRotation, 0.0f, 360.0f);
	ImGui::SliderFloat("Quad Scale", &qc.previewScale, 0.0f, 5.0f);	
}
void CircleUI(PreviewConfiguration& cc, float windowWidth)
{
	ImGui::Checkbox("Init Circle Preview", &cc.showPreview);
	ImGui::ColorEdit4("Circle Color", (float*)&cc.previewColor);
	ImGui::SliderFloat("Circle Radius", &cc.previewRadius, 0.0f, 1000.0f);
	ImGui::SliderFloat2("Circle Position", cc.previewPosition, 0.0f, windowWidth);
}
void TriangleUI(PreviewConfiguration& tc, float windowWidth)
{	
	ImGui::Checkbox("Init Triangle Preview", &tc.showPreview);
	ImGui::ColorEdit4("Triangle Color", (float*)&tc.previewColor);
	ImGui::SliderFloat2("Triangle Position", tc.previewPosition, 0.0f, windowWidth);
	ImGui::SliderFloat2("Triangle Size", tc.previewSize, 0.0f, 1000.0f);
	ImGui::SliderFloat("Triangle Scale", &tc.previewScale, 0.0f, 5.0f);
}
void ImageUI(PreviewConfiguration& imgc, ap::Window* win)
{	
	ImGui::SliderFloat2("Image Position", imgc.previewPosition, 0.0f, win->getWindowSize().x);
	ImGui::SliderFloat2("Image Size", imgc.previewSize, 0.0f, 1000.0f);
	ImGui::SliderFloat("Image Rotation", &imgc.previewRotation, 0.0f, 360.0f);
	ImGui::SliderFloat("Image Scale", &imgc.previewScale, 0.0f, 5.0f);
	if (ImGui::Button("Load Image From File"))
	{
		std::string filepath = ap::FileDialog::OpenFile("Text Files\0*.txt\0Any File\0*.*\0", win);
		if (!filepath.empty())
		{
			imgc.filepath = filepath;
			imgc.showPreview = true;
			// load it once just to get the height and width
			ap::Texture temp(filepath);
			imgc.previewSize[0] = temp.Width();
			imgc.previewSize[1] = temp.Height();
		}
	}
}
void CustomShapeUI(CustomShape& cs, ap::Window* win)
{
	float offset = 15.0f;
	ImGui::Checkbox("Init Custom Shape Preview", &cs.showPreview);	
	ImGui::DragFloat2("Master Position", (float*)&cs.movementVector, 1.0f, -20, 20);
	if (ImGui::ColorEdit4("Master Color", (float*)&cs.color))
		for (int i = 0; i < cs.vertexCount; i++)
			cs.vertices[i].color = cs.color;
	ImGui::SliderInt("Select Active Index", &cs.activeIndex, 0, cs.vertexCount-1);
	ImGui::SliderFloat2("Vertex Position", (float*)&cs.vertices[cs.activeIndex].position, 0.0f, win->Width());
	ImGui::ColorEdit4("Vertex Color", (float*)&cs.vertices[cs.activeIndex].color);
	if (ImGui::SliderInt("Vertex Count", &cs.vertexCount, 0, 32))
		cs.reset();
}
int main()
{	
	const char* glsl_version = "#version 330";  // will be added to AP2DGL some day
	ap::Window window(WIDTH, HEIGHT, "window");
	glfwSwapInterval(1);  // enable vsync so the application doesn't make the cpu run at full speed
	ap::Renderer renderer(&window);	
	std::vector<Drawable*> drawables;	
	Line currentBuffer;
	bool drawing = false;
	ap::Clock drawClock; // to limit the amount of verticies the renderer has to draw
	ap::Texture penTexture("assets/textures/pen.png");
	ap::Texture eraserTexture("assets/textures/eraser.png");
	bool isDrawSelected = true;
	bool isPenEnabled = true;
	ap::Quad pen;
	pen.setTexture(&penTexture);
	pen.setSize(ap::Vec2f(40.0f, 40.0f));
	float lineWidth = 1.0f;
	// ui is not handled by AP2DGL but allows ImGui to be used easily because it is OpenGL & GLFW based
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsLight();
	ImGui_ImplGlfw_InitForOpenGL(window.glfw_ptr(), false);
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImVec4 renderClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImVec4 userDrawColor    = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);	
	enum ConfigSlot { QUAD, CIRCLE, TRIANGLE, IMAGE, CUSTOM };
	std::array<PreviewConfiguration, 5> configurations;	
	std::vector<std::future<void>> futures;
	SaveScreenShotData saveAreaData(&window);
	CustomShape shapePreview;
	while (window.isOpen())
	{		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();		
		ShowMainMenuBar(&window, renderer, saveAreaData, drawables);
		pen.setPosition({ window.getMousePosition().x + 17.0f, window.getMousePosition().y - 17.0f });  // +- 17 to offset the texture
		glClearColor(renderClearColor.x, renderClearColor.y, renderClearColor.z, renderClearColor.w);
		{
			ImGui::Begin("Edit & Create!");            
			if (isDrawSelected)
			{
				if (ImGui::Button("Erase"))
				{
					isDrawSelected = false;
					pen.setTexture(&eraserTexture);
				}
			}
			else
			{
				if (ImGui::Button("Draw"))
				{
					isDrawSelected = true;
					pen.setTexture(&penTexture);
				}
			}
			ImGui::SameLine();
			if (isPenEnabled)
			{
				if (ImGui::Button("Disable Pen"))
					isPenEnabled = false;
			}
			else if (ImGui::Button("Enable Pen"))
				isPenEnabled = true;
			
			ImGui::Text("Adjust Drawing Width");
			ImGui::SliderFloat("", &lineWidth, 0.0f, 10.0f);    			
			ImGui::Text("Choose Drawing Color");
			ImGui::ColorEdit3("  ", (float*)&userDrawColor);
			ImGui::Text("Adjust BackGround Clear Color");
			if (ImGui::ColorEdit3(" ", (float*)&renderClearColor))  
				futures.push_back(std::async(std::launch::async, [&] { updateEraseVerticies(drawables, renderClearColor); }));							
			// erasing works by literally updating the color of every vertex of every line used for erasing to the current render clear color
			// Normally this would crash when there is a lot of drawables and lines because it was running on one thread and
			// it couldn't handle it. Using std::async to move the updating of the verticies to a different thread frees up the main
			// thread so it doesn't crash. Works suprisingly well
			if (ImGui::CollapsingHeader("Make A Quad!"))
			{
				PreviewConfiguration& qpc = configurations[ConfigSlot::QUAD];
				QuadUI(qpc, window.getWindowSize().x);
				if (qpc.previewDrawable == nullptr 
					&& qpc.showPreview)
				{
					qpc.previewDrawable = new Quad;
					qpc.previewDrawable->type = Type::QUAD;					
				}
				else if (qpc.previewDrawable != nullptr && qpc.showPreview)
				{
					Quad* q = dynamic_cast<Quad*>(qpc.previewDrawable);
					if (q != nullptr)
					{
						q->quad.setColor(convertVector4(qpc.previewColor));
						q->quad.setPosition(convertVector2(qpc.previewPosition));
						q->quad.setSize(convertVector2(qpc.previewSize));
						q->quad.setSize({ q->quad.getSize().x * qpc.previewScale, q->quad.getSize().y * qpc.previewScale });
						q->quad.rotate(qpc.previewRotation);						
					}
				}
				if (ImGui::Button("Create Quad") && qpc.showPreview)
				{
					Quad* newQuad = makeQuad(qpc.previewDrawable);
					drawables.push_back(newQuad);
					qpc.reset();
				}				
			}
			if (ImGui::CollapsingHeader("Make A Circle!"))
			{
				PreviewConfiguration& cpc = configurations[ConfigSlot::CIRCLE];
				CircleUI(cpc, window.getWindowSize().x);
				if (cpc.previewDrawable == nullptr
					&& cpc.showPreview)
				{
					cpc.previewDrawable = new Circle;
					cpc.previewDrawable->type = Type::CIRCLE;
				}
				else if (cpc.previewDrawable != nullptr	&& cpc.showPreview)
				{
					Circle* c = dynamic_cast<Circle*>(cpc.previewDrawable);
					if (c != nullptr)
					{
						c->circle.setColor(convertVector4(cpc.previewColor));
						c->circle.setPosition(convertVector2(cpc.previewPosition));
						c->circle.setRadius(cpc.previewRadius);
					}
				}
				if (ImGui::Button("Create Circle") && cpc.showPreview)
				{					
					Circle* newCircle = makeCircle(cpc.previewDrawable);					
					drawables.push_back(newCircle);		
					cpc.reset();
				}
			}
			if (ImGui::CollapsingHeader("Create A Triangle!"))
			{
				PreviewConfiguration& tpc = configurations[ConfigSlot::TRIANGLE];
				TriangleUI(tpc, window.getWindowSize().x);
				if (tpc.previewDrawable == nullptr && tpc.showPreview)
				{
					tpc.previewDrawable = new Triangle;
					tpc.previewDrawable->type = Type::TRIANGLE;
				}
				else if (tpc.previewDrawable != nullptr && tpc.showPreview)
				{
					Triangle* t = dynamic_cast<Triangle*>(tpc.previewDrawable);
					if (t != nullptr)
					{
						t->triangle.setColor(convertVector4(tpc.previewColor));
						t->triangle.setPosition(convertVector2(tpc.previewPosition));
						t->triangle.setSize(convertVector2(tpc.previewSize));
						t->triangle.setSize({ t->triangle.getSize().x * tpc.previewScale, t->triangle.getSize().y * tpc.previewScale });
					}
				}
				if (ImGui::Button("Create Triangle") && tpc.showPreview)
				{
					Triangle* newTri = makeTriangle(tpc.previewDrawable);
					drawables.push_back(newTri);
					tpc.reset();
				}
			}			
			if (ImGui::CollapsingHeader("Create An Image!"))
			{
				PreviewConfiguration& imgc = configurations[ConfigSlot::IMAGE];
				ImageUI(imgc, &window);				
				if (imgc.previewDrawable == nullptr && imgc.showPreview)
				{
					imgc.previewDrawable = new Image(imgc.filepath.c_str());
					imgc.previewDrawable->type = Type::IMAGE;
				}
				else if (imgc.previewDrawable != nullptr && imgc.showPreview)
				{
					Image* img = dynamic_cast<Image*>(imgc.previewDrawable);
					if (img != nullptr)
					{
						img->image.setSize(convertVector2(imgc.previewSize));
						img->image.setPosition(convertVector2(imgc.previewPosition));
						img->image.rotate(imgc.previewRotation);
						img->image.setSize({ img->image.getSize().x * imgc.previewScale, img->image.getSize().y * imgc.previewScale });
					}
				}
				if (ImGui::Button("Create Image") && imgc.showPreview)
				{					
					Image* img = makeImage(imgc.previewDrawable);
					drawables.push_back(img);
					imgc.reset();
				}
			}			
			if (ImGui::CollapsingHeader("Create Custom Shape!"))
			{
				CustomShapeUI(shapePreview, &window);
				
				if (shapePreview.showPreview)
				{
					shapePreview.update();
				}
				if (ImGui::Button("Create Shape"))
				{
					CustomShape* cs = new CustomShape(shapePreview);
					drawables.push_back(cs);
					shapePreview.showPreview = false;
					shapePreview.reset();
				}
			}
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
				1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}	
		if ((ap::Window::isKeyPressed(ap::key::AP_KEY_LEFT_CONTROL) || ap::Window::isKeyPressed(ap::key::AP_KEY_RIGHT_CONTROL))
			&& ap::Window::isKeyPressed(ap::key::AP_KEY_Z) && drawClock.GetElapsedTimeAsMilliseconds() >= 300.0f)
		{
			delete drawables.back();
			drawables.pop_back();
			drawClock.Restart();
		}			
		if (ap::Window::isMouseButtonPressed(ap::AP_MOUSE_BUTTON_LEFT) && !ImGui::IsAnyItemActive()
			&& drawClock.GetElapsedTimeAsMilliseconds() >= 10.0f && isPenEnabled)
		{					
			currentBuffer.buffer.SetWidth(lineWidth);
			drawing = true;
			if (isDrawSelected)
				currentBuffer.buffer.Push(ap::Vertex(window.getMousePosition(), convertVector4(userDrawColor)));			
			else
				currentBuffer.buffer.Push(ap::Vertex(window.getMousePosition(), convertVector4(renderClearColor)));
			drawClock.Restart();
		}		
		if (!ap::Window::isMouseButtonPressed(ap::AP_MOUSE_BUTTON_LEFT) && drawing)
		{				
			currentBuffer.type = Type::LINE;
			if (isDrawSelected)
				currentBuffer.buffer.setType(DrawType::DRAW);
			else if (!isDrawSelected)
				currentBuffer.buffer.setType(DrawType::ERASE);
			Line* temp = new Line(currentBuffer);
			drawables.push_back(temp);			
			currentBuffer.buffer.Clear();
			drawing = false;
		}	
		defineSaveCanvasArea(saveAreaData, window);		
		ImGui::Render();
		renderer.ClearRenderBuffer();				
		for (int i = 0; i < drawables.size(); i++)
			SubmitDrawableRenderData(drawables[i], renderer);
		currentBuffer.buffer.Draw(renderer);
		for (auto& c : configurations)
			if(c.showPreview)
				SubmitDrawableRenderData(c.previewDrawable, renderer);
		if (shapePreview.showPreview)
			renderCustomShape(shapePreview, renderer);
		if (saveAreaData.areaDefined || saveAreaData.definingArea)
			saveAreaData.draw(renderer);
		if (!ImGui::IsAnyWindowHovered())
			renderer.Draw(&pen);
		renderer.onUpdate();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		window.onUpdate();
	}
	for (auto& d : drawables)
		delete d;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
