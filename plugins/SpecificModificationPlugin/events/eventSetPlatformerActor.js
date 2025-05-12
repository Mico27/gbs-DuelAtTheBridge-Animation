const l10n = require("../helpers/l10n").default;

export const id = "EVENT_SET_PLATFORMER_ACTOR";
export const name = "Set platformer actor";
export const groups = ["EVENT_GROUP_ACTOR"];

export const autoLabel = (fetchArg) => {
  return `Set platformer actor`;
};

export const fields = [  
  {
    key: "actorId",
    label: l10n("ACTOR"),
    description: l10n("FIELD_ACTOR_DEACTIVATE_DESC"),
    type: "actor",
    defaultValue: "$self$",
  },
  {
    key: "playerId",
    label: "Player id",
    description: "Player id",
    type: "value",
     defaultValue: {
          type: "number",
          value: 0,
        },
  }
];

export const compile = (input, helpers) => {
  const { _callNative, _stackPush, _stackPop, _addComment, _declareLocal, variableSetToScriptValue, setActorId } = helpers;
  
  const tmp0 = _declareLocal("tmp0", 1, true);
  const tmp1 = _declareLocal("tmp1", 1, true);
    
  setActorId(tmp0, input.actorId);
  variableSetToScriptValue(tmp1, input.playerId);
    
  _addComment("Set platformer actor");
    
  _stackPush(tmp1);
  _stackPush(tmp0);
  		
  _callNative("vm_set_platformer_actor");
  _stackPop(2);   
  
};
