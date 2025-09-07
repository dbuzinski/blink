classdef tResponse < matlab.unittest.TestCase
    methods(Test)
        function testResponseCreation(testCase)
            resp = blink.Response();
            
            testCase.verifyEqual(resp.StatusCode, int16(200));
            testCase.verifyEqual(resp.Data, char.empty(1, 0));
        end
    end
end
