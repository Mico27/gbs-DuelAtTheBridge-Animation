export const id = "EVENT_HELPER_WOBBLE_EFFECT";
export const name = "Screen special effect";
export const groups = ["misc"];

export const autoLabel = (fetchArg) => {
  return `Screen special effect`;
};

export const fields = [
  {
    key: `lcd_effect`,
    label: "Enabled",
    type: "value",
    width: "50%",
    defaultValue: {
      type: "number",
      value: 0,
    },
  },
];

export const compile = (input, helpers) => {
  const { _callNative, _stackPush, _stackPop, _declareLocal, variableSetToScriptValue } = helpers;
  
  const tmp0 = _declareLocal("tmp_0", 1, true);
    
  variableSetToScriptValue(tmp0, input.lcd_effect);
   
  _stackPush(tmp0);
  		
  _callNative("set_temp_lcd_effect"); 
  _stackPop(1);  
  
};
