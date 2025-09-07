classdef tBlink < matlab.unittest.TestCase
    methods(Test)
        function testFrameworkInitialization(testCase)
            try
                app = blink.App();
                testCase.verifyInstanceOf(app, 'blink.App');
            catch ME
                testCase.verifyFail(['Framework initialization failed: ' ME.message]);
            end
        end
        
        function testBasicRouteHandling(testCase)
            app = blink.App();
            app.get('/test', @(req, resp) setfield(resp, 'Data', 'success'));
            
            testCase.verifyEqual(length(app.Routes), 1);
            testCase.verifyEqual(app.Routes(1).HttpMethod, 'GET');
            testCase.verifyEqual(app.Routes(1).Path, '/test');
        end
        
        function testStaticFilesConfiguration(testCase)
            app = blink.App();
            app.StaticFiles = "test_static";
            app.StaticRoute = "/assets";
            
            testCase.verifyEqual(app.StaticFiles, "test_static");
            testCase.verifyEqual(app.StaticRoute, "/assets");
        end
    end
end