-- Main Table
local library = {};

-- Services
local TweenService      = game:GetService("TweenService");
local UserInputService  = game:GetService("UserInputService");
local Mouse             = game:GetService("Players").LocalPlayer:GetMouse();
local CoreGui           = game:GetService("CoreGui");

-- Tween Func
local function Tween(obj, time, tab)
    if (not obj or not time or not tab) then return; end;

    TweenService:Create(obj, TweenInfo.new(time), tab):Play();
end;

-- Instance Creation
function library:Create(class, prop)
    local object    = Instance.new(class);
    prop            = typeof(prop) == "table" and prop or {};

    if (class == "ScreenGui" and syn) then
        syn.protect_gui(object);
    end;

    for i, v in next, prop do
        object[i] = v;
    end;

    return object;
end;

-- check
if (_G.MaxxHub) then
    _G.MaxxHub:Destroy();
end;

-- Main UI Build
do
    -- Screen Gui
    local MainScreen = library:Create("ScreenGui", {
        Name            = "Wayhub";
        DisplayOrder    = 1;
        Parent          = game:GetService("CoreGui");
        ResetOnSpawn    = false;
        Enabled         = true;
    });
    
    _G.MaxxHub = MainScreen
    -- MainFrame
    local MainFrame = library:Create("Frame", {
        Name                = "WayFrame";
        Parent              = MainScreen;
        Size                = UDim2.new(0, 538, 0, 310);
        Position            = UDim2.new(0.228, 0, 0.252, 0);
        Active              = true;
        Draggable           = true;
        ZIndex              = 2;
        BackgroundColor3    = Color3.fromRGB(24, 24, 24);
        BorderSizePixel     = 0;
    });

    -- UICorner
    local MainCorner = library:Create("UICorner", {
        CornerRadius = UDim.new(0, 6);
        Parent       = MainFrame;
    });

    -- ImageLabel
    local Glow = library:Create("ImageLabel", {
        Name                    = "Glow";
        Image                   = "rbxassetid://5028857084";
        Parent                  = MainFrame;
        ImageColor3             = Color3.fromRGB(25, 25, 25);
        Position                = UDim2.new(0, -15, 0, -15);
        BorderColor3            = Color3.fromRGB(27, 42, 53);
        Size                    = UDim2.new(1, 30, 1, 30);
        ImageTransparency       = 0;
        BorderSizePixel         = 1;
        BackgroundTransparency  = 1;
    });

    -- MainCorner Thingy
    local MainCorner = library:Create("UICorner", {
        Name            = "MainCorner";
        Parent          = MainFrame;
        CornerRadius    = UDim.new(0, 6);
    });

    -- Title TextLabel
    local TitleStuff = library:Create("TextLabel", {
        Text                        = "Title";
        Parent                      = MainFrame;
        BackgroundColor3            = Color3.fromRGB(115, 129, 255);
        BackgroundTransparency      = 0.9;
        BorderColor3                = Color3.fromRGB(27, 42, 53);
        Position                    = UDim2.new(0.031, 0, 0.048, 0);
        Size                        = UDim2.new(0, 100, 0, 25);
        Text                        = "<b>MAXX HUB</b>";
        TextColor3                  = Color3.fromRGB(115,129,255);
        TextSize                    = 20;
        RichText                    = true;
        ZIndex                      = 2;
        Font                        = Enum.Font.SourceSansSemibold;
    });

    -- TitleCorner
    local TitleCorner = library:Create("UICorner",{
        Name            = "TitleCorner";
        Parent          = TitleStuff;
        CornerRadius    = UDim.new(0, 6);
    });

    -- Tab Frame
    local Tabs = library:Create("Frame", {
        Name                    = "Tabs";
        Parent                  = MainFrame;
        BackgroundTransparency  = 1;
        Position                = UDim2.new(0.246, 0, 0.026, 0);
        Size                    = UDim2.new(0, 405, 0, 39);
    });

    -- UI List Tab
    local UIListTab = library:Create("UIListLayout", {
        Parent              = Tabs;
        FillDirection       = Enum.FillDirection.Horizontal;
        SortOrder           = Enum.SortOrder.LayoutOrder;
        VerticalAlignment   = Enum.VerticalAlignment.Center;
        Padding             = UDim.new(0, 6);
    });

    -- Containers Folder
    local Containers = library:Create("Folder", {
        Parent = MainFrame;
    });

    -- UI Lib Functions
    do
        -- Add Tab
        function library:AddTab(name)
            -- TextButton Creation
            local Tab = library:Create("TextButton", {
                Name                = name;
                Parent              = Tabs;
                Text                = name;
                Font                = Enum.Font.SourceSansSemibold;
                TextSize            = 17;
                TextColor3          = Color3.fromRGB(255, 255, 255);
                AutoButtonColor     = false;
                BorderSizePixel     = 0;
                BackgroundColor3    = Color3.fromRGB(115, 129, 255);
                Size                = UDim2.new(0, 60, 0, 25);
                Position            = UDim2.new(0, 0, 0.179, 0);
                ZIndex              = 3;
            });

            -- Corner Creation
            local TabCorner = library:Create("UICorner", {
                Name        = ("UICorner" .. name);
                Parent      = Tab;
                CornerRadius= UDim.new(0, 6);
            });

            -- Glow Creation
            local Glow = library:Create("ImageLabel", {
                Name                    = ("Glow" .. name);
                Image                   = "rbxassetid://5028857084";
                Parent                  = Tab;
                ImageColor3             = Color3.fromRGB(25, 25, 25);
                Position                = UDim2.new(0, -15, 0, -15);
                BorderColor3            = Color3.fromRGB(27, 42, 53);
                Size                    = UDim2.new(1, 30, 1, 30);
                ImageTransparency       = 0;
                BorderSizePixel         = 1;
                BackgroundTransparency  = 1;
            });

            -- Container Creation
            local Container = library:Create("Frame", {
                Name                    = "Container";
                Parent                  = Containers;
                BackgroundColor3        = Color3.fromRGB(29, 29, 29);
                Position                = UDim2.new(0.018, 0, 0.177, 0);
                Size                    = UDim2.new(0, 518, 0, 246);
                ZIndex                  = 2;
                BackgroundTransparency  = not Containers:FindFirstChild("Container") and 0 or 1;
                BorderSizePixel         = 0;
            });

            -- Container UICorner
            local ContainerCorner = library:Create("UICorner", {
                Parent          = Container;
                CornerRadius    = UDim.new(0, 6);
            });

            -- Glow Creation Container
            local GlowContainer = library:Create("ImageLabel", {
                Name                    = ("Glow" .. name);
                Image                   = "rbxassetid://5028857084";
                Parent                  = Container;
                ImageColor3             = Color3.fromRGB(25, 25, 25);
                Position                = UDim2.new(0, -15, 0, -15);
                BorderColor3            = Color3.fromRGB(27, 42, 53);
                Size                    = UDim2.new(1, 30, 1, 30);
                ImageTransparency       = 0;
                BorderSizePixel         = 1;
                BackgroundTransparency  = 1;
            });

            -- Object Frame
            local ObjectsFrame = library:Create("ScrollingFrame", {
                Name                    = ("Object" .. name);
                Parent                  = Container;
                BackgroundTransparency  = 1;
                Size                    = UDim2.new(0, 518, 0, 246);
                Position                = UDim2.new(0, 0, 0, 0);
                ZIndex                  = 3;
                Active                  = true;
                BorderSizePixel         = 0;
                ScrollBarThickness      = 5;
                TopImage                = "";
                BottomImage             = "";
                MidImage                = "";
                Active                  = true;
                Draggable               = true;
            });

            -- UIList For Objects
            library:Create("UIListLayout", {
                Parent              = ObjectsFrame;
                Padding             = UDim.new(0, 10);
                FillDirection       = Enum.FillDirection.Vertical;
                SortOrder           = Enum.SortOrder.LayoutOrder;
                VerticalAlignment   = Enum.VerticalAlignment.Top;
            });

            -- UIPadding For Objects
            library:Create("UIPadding", {
                Parent      = ObjectsFrame;
                PaddingTop  = UDim.new(0, 10);
            });

            -- Cool Tab Effects
            Tab.MouseEnter:Connect(function()
                Tween(Tab, .2, {
                    BackgroundColor3 = Color3.fromRGB(115, 129, 180)
                });
            end);

            Tab.MouseLeave:Connect(function()
                Tween(Tab, .2, {
                    BackgroundColor3 = Color3.fromRGB(115, 129, 255)
                });
            end);

            -- ButtonClick
            Tab.MouseButton1Click:Connect(function()
                for i, v in next, Containers:GetChildren() do
                    v.Visible = false;
                end;

                wait(0.2);

                Container.Visible = true;
            end);

            -- UI Object Functions
            local T2 = {};

            function T2:Show()
                for i, v in next, Containers:GetChildren() do
                    if v == Container then
                        v.Visible = true;
                    else
                        v.Visible = false;
                    end
                end;
            end
            -- Toggle Func
            function T2:CreateToggle(options)
                options = options or {
                    Text        = "";
                    Callback    = function()
                    end;
                    Enabled     = false;
                };

                local ToggleButton = library:Create("TextButton", {
                    Name                    = ("Toggle" .. options.Text);
                    Parent                  = ObjectsFrame;
                    Text                    = ("    " .. options.Text);
                    TextColor3              = Color3.fromRGB(255, 255, 255);
                    TextSize                = 15;
                    Active                  = true;
                    AutoButtonColor         = false;
                    BackgroundColor3        = Color3.fromRGB(34, 34, 34);
                    BackgroundTransparency  = 0;
                    Size                    = UDim2.new(0, 498, 0, 30);
                    Font                    = Enum.Font.SourceSansSemibold;
                    TextXAlignment          = Enum.TextXAlignment.Left;
                    TextYAlignment          = Enum.TextYAlignment.Center;
                    Active                  = true;
                    Draggable               = true;
                    ZIndex                  = 3;
                });

                -- UICorner Creation
                library:Create("UICorner", {
                    Parent          = ToggleButton;
                    CornerRadius    = UDim.new(0, 6);
                });

                local FrameStuff = library:Create("Frame", {
                    Parent              = ToggleButton;
                    Position            = UDim2.new(0.918, 0, 0.16, 0);
                    Size                = UDim2.new(0, 35, 0, 20);
                    BackgroundColor3    = Color3.fromRGB(29, 29, 29);
                    BorderSizePixel     = 0;
                    ZIndex              = 3;
                });

                -- UICorner Creation
                library:Create("UICorner", {
                    Parent          = FrameStuff;
                    CornerRadius    = UDim.new(0, 6);
                });

                local FinalFrameStuff = library:Create("Frame", {
                    Parent              = FrameStuff;
                    Position            = options.Enabled and UDim2.new(0.5, 0, 0.085, 0) or UDim2.new(0.15, 0, 0.085, 0);
                    Size                = UDim2.new(0, 15, 0, 15);
                    BackgroundColor3    = options.Enabled and Color3.fromRGB(115, 129, 255) or Color3.fromRGB(255, 10, 14);
                    BorderSizePixel     = 0;
                    ZIndex              = 3;
                });

                -- UICorner Creation
                library:Create("UICorner", {
                    Parent          = FinalFrameStuff;
                    CornerRadius    = UDim.new(0, 6);
                });

                -- Connection
                local state = options.Enabled or false;

                ToggleButton.MouseButton1Click:Connect(function()
                    state = not state;

                    if (state) then
                        Tween(FinalFrameStuff, 0.2, {
                            BackgroundColor3    = Color3.fromRGB(115, 129, 255);
                            Position            = UDim2.new(0.5, 0, 0.085, 0);
                        });
                    else
                        Tween(FinalFrameStuff, 0.2, {
                            BackgroundColor3    = Color3.fromRGB(255, 10, 14);
                            Position            = UDim2.new(0.15, 0, 0.085, 0);
                        });
                    end;

                    options.Callback(state);
                end);
            end;

            -- Button Func
            function T2:CreateButton(options)
                options = options or {
                    Text        = "";
                    Callback    = function()
                    end;
                };

                local ButtonStuff = library:Create("TextButton", {
                    Name                    = ("Button" .. options.Text);
                    Parent                  = ObjectsFrame;
                    Text                    = ("    " .. options.Text);
                    TextSize                = 15;
                    TextColor3              = Color3.fromRGB(255, 255, 255);
                    Font                    = Enum.Font.SourceSansSemibold;
                    Parent                  = ObjectsFrame;
                    AutoButtonColor         = false;
                    Size                    = UDim2.new(0, 498, 0, 30);
                    BackgroundColor3        = Color3.fromRGB(34, 34, 34);
                    TextXAlignment          = Enum.TextXAlignment.Left;
                    TextYAlignment          = Enum.TextYAlignment.Center;
                    ZIndex                  = 3;
                });

                -- UICorner Creation
                library:Create("UICorner", {
                    Parent          = ButtonStuff;
                    CornerRadius    = UDim.new(0, 6);
                });

                ButtonStuff.MouseEnter:Connect(function()
                    Tween(ButtonStuff, .2, {
                        BackgroundColor3 = Color3.fromRGB(31, 31, 31);
                    })
                end)

                ButtonStuff.MouseLeave:Connect(function()
                    Tween(ButtonStuff, .2, {
                        BackgroundColor3 = Color3.fromRGB(34, 34, 34);
                    })
                end)
                -- Callback
                ButtonStuff.MouseButton1Click:Connect(function()
                    CircleAnim(ButtonLabel, ThisTheme.ButtonAccent, ThisTheme.Button)
                    options.Callback();
                end);
            end;

            -- TextBox Func
            function T2:CreateTextbox(options)
                options = options or {
                    Text        = "";
                    Callback    = function()
                    end;
                    StartText   = "";
                };

                local TextButton = library:Create("TextButton", {
                    Name                    = ("TextButton" .. options.Text);
                    Parent                  = ObjectsFrame;
                    Text                    = ("    " .. options.Text);
                    TextSize                = 15;
                    AutoButtonColor         = false;
                    TextColor3              = Color3.fromRGB(255, 255, 255);
                    Font                    = Enum.Font.SourceSansSemibold;
                    Parent                  = ObjectsFrame;
                    Size                    = UDim2.new(0, 498, 0, 30);
                    BackgroundColor3        = Color3.fromRGB(34, 34, 34);
                    TextXAlignment          = Enum.TextXAlignment.Left;
                    TextYAlignment          = Enum.TextYAlignment.Center;
                    AutoLocalize            = true;
                    ZIndex                  = 3;
                });

                -- UICorner Creation
                library:Create("UICorner", {
                    Parent          = TextButton;
                    CornerRadius    = UDim.new(0, 6);
                });

                -- TextBox
                local TextBox = library:Create("TextBox", {
                    Name                = ("Textbox" .. options.Text);
                    Text                = options.StartText and options.StartText or "";
                    Parent              = TextButton;
                    Position            = UDim2.new(0.805, 0, 0.2, 0);
                    Size                = UDim2.new(0, 91, 0, 18);
                    BackgroundColor3    = Color3.fromRGB(29, 29, 29);
                    TextColor3          = Color3.fromRGB(255, 255, 255);
                    TextSize            = 14;
                    Font                = Enum.Font.SourceSansSemibold;
                    TextEditable        = true;
                    Active              = true;
                    ZIndex              = 3;
                });

                -- UICorner Creation
                library:Create("UICorner", {
                    Parent          = TextBox;
                    CornerRadius    = UDim.new(0, 6);
                });

                -- Connection
                TextBox.FocusLost:Connect(function()
                    pcall(options.Callback, TextBox.Text);
                end);
            end;

            -- Slider Func
            function T2:CreateSlider(options)
                options = options or {
                    Text        = "";
                    Min         = 0;
                    Max         = 0;
                    Def         = 0;
                    Callback    = function()
                    end;
                };
        
                local Slider = library:Create("TextButton", {
                    Parent              = ObjectsFrame;
                    BackgroundColor3    = Color3.fromRGB(34, 34, 34);
                    Size                = UDim2.new(0, 498, 0, 30);
                    AutoButtonColor     = false;
                    Font                = Enum.Font.SourceSansSemibold;
                    Text                = "";
                    TextColor3          = Color3.fromRGB(255, 255, 255);
                    TextSize            = 15;
                    ZIndex              = 2;
                });
        
                local SliderValue = library:Create("TextLabel", {
                    Parent                  = Slider;
                    BackgroundColor3        = Color3.fromRGB(255, 255, 255);
                    BackgroundTransparency  = 1;
                    Size                    = UDim2.new(0, 498, 0, 30);
                    Font                    = Enum.Font.SourceSansSemibold;
                    Text                    = ("    " .. options.Text .. " : " .. options.Def .. "%");
                    TextColor3              = Color3.fromRGB(255, 255, 255);
                    TextSize                = 15;
                    TextXAlignment          = Enum.TextXAlignment.Left;
                    TextYAlignment          = Enum.TextYAlignment.Center;
                    ZIndex                  = 2;
                });
        
                -- UICorner Creation
                library:Create("UICorner", {
                    CornerRadius    = UDim.new(0, 6);
                    Parent          = Slider;
                });
        
                local SliderInner = library:Create("Frame", {
                    Parent                  = Slider;
                    BackgroundColor3        = Color3.fromRGB(48, 50, 59);
                    Position                = UDim2.new(0.126, -60, 0.467, -12);
                    Size                    = UDim2.new(((options.Def or options.Min) - options.Min) / (options.Max - options.Min), 0, 0, 25);
                    BackgroundTransparency  = 0.5;
                    ZIndex                  = 2;
                });
        
                local SliderInnerMax = library:Create("Frame", {
                    Parent                  = Slider;
                    AnchorPoint             = Vector2.new(0.5, 0.5);
                    BackgroundColor3        = Color3.fromRGB(48, 50, 59);
                    BackgroundTransparency  = 1;
                    Position                = UDim2.new(0.498837203, 0, 0.791666687, -9);
                    Size                    = UDim2.new(0.15, 425, 0, 25);
                });
        
                -- UICorner Creation
                library:Create("UICorner", {
                    CornerRadius    = UDim.new(0, 6);
                    Parent          = SliderInner;
                });
        
                local Value;
        
                Slider.MouseButton1Down:Connect(function()
                    down                = true;
                    Value               = math.floor((((tonumber(options.Max) - tonumber(options.Min)) / 425) * SliderInner.AbsoluteSize.X) + tonumber(options.Min)) or 0;
                    SliderValue.Text    = "    " .. options.Text .. " : " .. tostring(Value) .. "%";

                    pcall(options.Callback, Value);
                    SliderInner:TweenSize(UDim2.new(0, math.clamp(Mouse.X - SliderInner.AbsolutePosition.X, 0, 425), 0, 25), Enum.EasingDirection.InOut, Enum.EasingStyle.Linear, 0.07);

                    while game:GetService("RunService").RenderStepped:wait() and down do
                        Value               = math.floor((((tonumber(options.Max) - tonumber(options.Min)) / 425) * SliderInner.AbsoluteSize.X) + tonumber(options.Min)) or 0;
                        SliderValue.Text    = "    " .. options.Text .. " : " .. tostring(Value) .. "%";

                        pcall(options.Callback, Value);
                        SliderInner:TweenSize(UDim2.new(0, math.clamp(Mouse.X - SliderInner.AbsolutePosition.X, 0, 425), 0, 25), Enum.EasingDirection.InOut, Enum.EasingStyle.Linear, 0.07);
                    end;
                end);
        
                UserInputService.InputEnded:Connect(function(key)
                    if (key.UserInputType == Enum.UserInputType.MouseButton1 and down) then
                        down                = false;
                        Value               = math.floor((((tonumber(options.Max) - tonumber(options.Min)) / 425) * SliderInner.AbsoluteSize.X) + tonumber(options.Min)) or 0;
                        SliderValue.Text    = "    " .. options.Text .. " : " .. tostring(Value) .. "%";

                        pcall(options.Callback, Value);
                        SliderInner:TweenSize(UDim2.new(0, math.clamp(Mouse.X - SliderInner.AbsolutePosition.X, 0, 425), 0, 25), Enum.EasingDirection.InOut, Enum.EasingStyle.Linear, 0.07);
                    end;
                end);
            end;

            -- Dropdown Func
            function T2:CreateDropdown(options)
                options = options or {
                    Text = "";
                    Callback = function()
        
                    end;
                    Options = {
                        "1";
                        "2";
                    };
                }
        
                local Dropdown = library:Create("TextButton", {
                    Parent                  = ObjectsFrame;
                    BackgroundColor3        = Color3.fromRGB(34, 34, 34);
                    Size                    = UDim2.new(0, 498, 0, 30);
                    Font                    = Enum.Font.SourceSansSemibold;
                    Text                    = ("    " .. options.Text);
                    AutoButtonColor         = false;
                    TextColor3              = Color3.fromRGB(255, 255, 255);
                    TextSize                = 15;
                    TextXAlignment          = Enum.TextXAlignment.Left;
                    ZIndex                  = 2;
                });
        
                -- UICorner Creation
                library:Create("UICorner", {
                    CornerRadius    = UDim.new(0, 6);
                    Parent          = Dropdown;
                });
        
                local Arrow = library:Create("ImageLabel", {
                    Parent                      = Dropdown;
                    BackgroundTransparency      = 1;
                    Position                    = UDim2.new(0.938, 0, 0.12, 0);
                    Size                        = UDim2.new(0, 20, 0, 20);
                    ZIndex                      = 2;
                    Image                       = "rbxassetid://3926305904";
                    ImageRectOffset             = Vector2.new(404, 284);
                    ImageRectSize               = Vector2.new(36, 36);
                });
        
                local DropdownList = library:Create("Frame", {
                    Parent                  = Dropdown;
                    BackgroundColor3        = Color3.fromRGB(34, 34, 34);
                    BorderSizePixel         = 0;
                    Position                = UDim2.new(0, 0, 0.899999976, 0);
                    Size                    = UDim2.new(0, 498, 0, 0);
                    ClipsDescendants        = true;
                    ZIndex                  = 5;
                });
        
                -- UICorner Creation
                library:Create("UIListLayout", {
                    Parent                  = DropdownList;
                    HorizontalAlignment     = Enum.HorizontalAlignment.Center;
                    SortOrder               = Enum.SortOrder.LayoutOrder;
                });
        
                local BodyYSize     = 5;
                local Opened        = false;
        
                for i, v in next, options.Options do
                    local Button = library:Create("TextButton", {
                        Parent                  = DropdownList;
                        BackgroundColor3        = Color3.fromRGB(48, 50, 59);
                        Size                    = UDim2.new(0, 417, 0, 25);
                        Font                    = Enum.Font.SourceSans;
                        TextColor3              = Color3.fromRGB(255, 255, 255);
                        TextSize                = 14;
                        AutoButtonColor         = false;
                        Text                    = v;
                        BackgroundTransparency  = 1;
                        BorderSizePixel         = 0;
                        ZIndex                  = 6;
                    });
        
                    -- UICorner Creation
                    library:Create("UICorner", {
                        CornerRadius    = UDim.new(0, 6);
                        Parent          = Button;
                    });
        
                    Button.MouseEnter:Connect(function()
                        Tween(Button, 0.2, {
                            BackgroundTransparency = 0.5;
                        });
                    end);
        
                    Button.MouseLeave:Connect(function()
                        Tween(Button, 0.2, {
                            BackgroundTransparency  = 1;
                            BackgroundColor3        = Color3.fromRGB(48, 50, 59);
                        });
                    end);
        
                    Button.MouseButton1Click:Connect(function()
                        Opened = false;

                        Tween(DropdownList, 0.2, {
                            Size = UDim2.new(0, 498, 0, 0);
                        });

                        options.Callback(v);
                        Dropdown.Text = "    "..options.Text..": "..v
                    end);
        
                    BodyYSize = BodyYSize + 25;
                end;
        
                Dropdown.MouseButton1Click:Connect(function()
                    for i, v in next, Dropdown:GetChildren() do
                        if (v == DropdownList and v.Size == UDim2.new(0, 498, 0, BodyYSize)) then
                            Tween(v, 0.2, {
                                Size = UDim2.new(0, 498, 0, 0);
                            });
                        end;
                    end;
        
                    Opened = not Opened;

                    if (Opened) then
                        Tween(DropdownList, 0.2, {
                            Size = UDim2.new(0, 498, 0, BodyYSize)
                        });
                    else
                        Tween(DropdownList, 0.2, {
                            Size = UDim2.new(0, 498, 0, 0);
                        });
                    end;
                end);

                -- Funcs for drop downs
                local A2 = {};
        
                function A2:Add(Name)
                    local Button = library:Create("TextButton", {
                        Parent                  = DropdownList;
                        BackgroundColor3        = Color3.fromRGB(48, 50, 59);
                        Size                    = UDim2.new(0, 417, 0, 25);
                        Font                    = Enum.Font.SourceSans;
                        TextColor3              = Color3.fromRGB(255, 255, 255);
                        TextSize                = 14;
                        AutoButtonColor         = false;
                        Text                    = Name;
                        BackgroundTransparency  = 1;
                        BorderSizePixel         = 0;
                        ZIndex                  = 6;
                    });
        
                    -- UICorner Creation
                    library:Create("UICorner", {
                        CornerRadius    = UDim.new(0, 6);
                        Parent          = Button;
                    });
        
                    Button.MouseEnter:Connect(function()
                        Tween(Button, 0.2, {
                            BackgroundTransparency = 0.5;
                        });
                    end);
        
                    Button.MouseLeave:Connect(function()
                        Tween(Button, 0.2, {
                            BackgroundTransparency  = 1;
                            BackgroundColor3        = Color3.fromRGB(48, 50, 59);
                        });
                    end);
        
                    Button.MouseButton1Click:Connect(function()
                        Opened = false;

                        Tween(DropdownList, 0.2, {
                            Size = UDim2.new(0, 498, 0, 0);
                        });

                        options.Callback(Name);
                        Dropdown.Text = "    "..options.Text..": "..Name
                    end);
        
                    BodyYSize = BodyYSize + 25;
                    
                    if (Opened) then
                        Tween(DropdownList, 0.2, {Size = UDim2.new(0, 498, 0, BodyYSize)});
                    end;
                end;

                function A2:Remove(Name)
                    for i, v in next, DropdownList:GetChildren() do
                        if (v:IsA("TextButton") and v.Text:lower():find(Name)) then
                            v:Destroy();
                            BodyYSize = BodyYSize - 25;
                        end;
                    end;
            
                    if (Opened) then
                        Tween(DropdownList, 0.2, {Size = UDim2.new(0, 498, 0, BodyYSize)});
                    end;
                end;

                return A2;
            end;

            return T2;
        end;
    end;
    return library
end;
