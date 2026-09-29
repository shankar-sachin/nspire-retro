'use strict';
const teams = [
['NYG','New York Giants','NFC'],['GB','Green Bay Packers','NFC'],['SEA','Seattle Seahawks','NFC'],['KC','Kansas City Chiefs','AFC'],['BUF','Buffalo Bills','AFC'],['BAL','Baltimore Ravens','AFC'],['PHI','Philadelphia Eagles','NFC'],['DET','Detroit Lions','NFC'],['LAR','Los Angeles Rams','NFC'],['LAC','Los Angeles Chargers','AFC'],['MIA','Miami Dolphins','AFC'],['CLE','Cleveland Browns','AFC'],['CIN','Cincinnati Bengals','AFC'],['NYJ','New York Jets','AFC'],['SF','San Francisco 49ers','NFC'],['DAL','Dallas Cowboys','NFC'],['PIT','Pittsburgh Steelers','AFC'],['DEN','Denver Broncos','AFC'],['NE','New England Patriots','AFC'],['HOU','Houston Texans','AFC'],['TB','Tampa Bay Buccaneers','NFC'],['MIN','Minnesota Vikings','NFC'],['ATL','Atlanta Falcons','NFC'],['WAS','Washington Commanders','NFC'],['TEN','Tennessee Titans','AFC'],['IND','Indianapolis Colts','AFC'],['JAX','Jacksonville Jaguars','AFC'],['LV','Las Vegas Raiders','AFC'],['NO','New Orleans Saints','NFC'],['CAR','Carolina Panthers','NFC'],['ARI','Arizona Cardinals','NFC'],['CHI','Chicago Bears','NFC']
];
function filterTeams(conference) {
 if (!['ALL','AFC','NFC'].includes(conference)) throw new Error('Choose ALL, AFC, or NFC.');
 const shown = teams.filter(team => conference === 'ALL' || team[2] === conference);
 const list = document.querySelector('#teams'); list.replaceChildren();
 for (const [abbr,name] of shown) { const row=document.createElement('div'); row.className='team'; const code=document.createElement('b'); code.textContent=abbr; const label=document.createElement('span'); label.textContent=name; row.append(code,label); list.append(row); }
 document.querySelectorAll('[data-conference]').forEach(button=>{ const active=button.dataset.conference===conference; button.classList.toggle('active',active); button.setAttribute('aria-pressed',String(active)); });
 return {conference, count:shown.length, teams:shown.map(team=>team[1])};
}
document.querySelectorAll('[data-conference]').forEach(button=>button.addEventListener('click',()=>filterTeams(button.dataset.conference)));
filterTeams('ALL');
for (const name of ['Split','Sweep','Counter','Draw','Slant','Cross','Go','Out','Post','Screen']) { const item=document.createElement('span'); item.textContent=name; document.querySelector('#plays').append(item); }
if(document.modelContext?.registerTool){const lifecycle=new AbortController(); try{Promise.resolve(document.modelContext.registerTool({name:'filter_teams',description:'Show all teams or one conference in the league section.',inputSchema:{type:'object',properties:{conference:{type:'string',enum:['ALL','AFC','NFC']}},required:['conference'],additionalProperties:false},annotations:{readOnlyHint:false},execute(input){if(!input || typeof input.conference!=='string')throw new Error('Conference is required.');return filterTeams(input.conference);}}, {signal:lifecycle.signal})).catch(()=>{});}catch{}window.addEventListener('pagehide',()=>lifecycle.abort(),{once:true});}
